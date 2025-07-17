#include "Loader.hpp"
#include "Engine.hpp"

#include <fastgltf/util.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <stb_image.h>

#include <spdlog/spdlog.h>

#include <variant>

namespace ve::gltf {

Loader::Loader( ve::Engine& engine, const ve::MemoryAllocator& allocator )
    : m_engine{ engine }, m_memoryAllocator{ allocator } {}

std::optional< std::shared_ptr< ve::gltf::Scene > > Loader::load( const std::filesystem::path& path ) {
    spdlog::info( "Loading model: {}", path.string() );
    const auto asset{ getAsset( path ) };
    if ( !asset.has_value() )
        return std::nullopt;

    using Ratio = ve::DescriptorAllocator::PoolSizeRatio;
    constexpr std::array< Ratio, 3U > sizes{ Ratio{ vk::DescriptorType::eCombinedImageSampler, 3 },
                                             Ratio{ vk::DescriptorType::eUniformBuffer, 3 },
                                             Ratio{ vk::DescriptorType::eStorageBuffer, 1 } };

    const auto& logicalDevice{ m_engine.getLogicalDevice() };
    const uint32_t setsCount{ ve::utils::size( asset->materials ) };

    std::shared_ptr< ve::gltf::Scene > scene{ std::make_shared< ve::gltf::Scene >() };
    scene->path = path;

    if ( setsCount != 0U )
        scene->descriptorAllocator.emplace( logicalDevice, setsCount, sizes );

    scene->samplers.reserve( std::size( asset->samplers ) );
    std::ranges::for_each( asset.value().samplers, [ &scene, &logicalDevice ]( const auto& gltfSampler ) {
        scene->samplers.emplace_back( logicalDevice, gltfSampler );
    } );

    setNodesRalationship( asset.value(), *scene );

    return scene;
}

std::optional< ve::Image > Loader::loadImage( const fastgltf::Asset& asset, ve::gltf::Scene& scene,
                                              const fastgltf::Image& image ) {
    std::optional< ve::Image > newImage{};
    int width;
    int height;
    int nrChannels;

    const auto createTextureImage{ [ &width, &height, &newImage, this ]( stbi_uc *data ) {
        if ( data ) {
            vk::Extent2D imagesize{ static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) };
            newImage.emplace(
                m_engine.createImage( data, imagesize, vk::Format::eR8G8B8A8Srgb,
                                      vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled ) );
            stbi_image_free( data );
        } else {
            spdlog::error( "failed to load texture" );
        }
    } };

    const auto ignoreRestDataSource{ []( auto& ) {} };

    const auto handleURI{ [ & ]( const fastgltf::sources::URI& filePath ) {
        assert( filePath.fileByteOffset == 0 );
        assert( filePath.uri.isLocalPath() );

        const auto uriRelativePath{ filePath.uri.fspath() };
        const auto path( scene.path.parent_path() / uriRelativePath );

        stbi_uc *data{ stbi_load( path.string().c_str(), &width, &height, &nrChannels, STBI_rgb_alpha ) };
        createTextureImage( data );
    } };

    const auto handleVector{ [ & ]( const fastgltf::sources::Vector& vector ) {
        stbi_uc *data{ stbi_load_from_memory( reinterpret_cast< const stbi_uc * >( std::data( vector.bytes ) ),
                                              static_cast< int >( std::size( vector.bytes ) ), &width, &height,
                                              &nrChannels, STBI_rgb_alpha ) };
        createTextureImage( data );
    } };

    const auto handleBufferView{ [ & ]( const fastgltf::sources::BufferView& view ) {
        auto& bufferView{ asset.bufferViews.at( view.bufferViewIndex ) };
        auto& buffer{ asset.buffers.at( bufferView.bufferIndex ) };

        const auto handleBufferVector{ [ & ]( const fastgltf::sources::Vector& vector ) {
            stbi_uc *data{ stbi_load_from_memory(
                reinterpret_cast< const stbi_uc * >( std::data( vector.bytes ) ) + bufferView.byteOffset,
                static_cast< int >( bufferView.byteLength ), &width, &height, &nrChannels, STBI_rgb_alpha ) };
            createTextureImage( data );
        } };

        const auto handleBufferArray{ [ & ]( const fastgltf::sources::Array& array ) {
            stbi_uc *data{ stbi_load_from_memory(
                reinterpret_cast< const stbi_uc * >( std::data( array.bytes ) ) + bufferView.byteOffset,
                static_cast< int >( bufferView.byteLength ), &width, &height, &nrChannels, STBI_rgb_alpha ) };
            createTextureImage( data );
        } };

        std::visit( fastgltf::visitor{ handleBufferVector, handleBufferArray, ignoreRestDataSource }, buffer.data );
    } };

    std::visit( fastgltf::visitor{ handleURI, handleVector, handleBufferView, ignoreRestDataSource }, image.data );

    return newImage;
}

void Loader::loadImages( const fastgltf::Asset& asset, ve::gltf::Scene& scene ) {
    scene.images.reserve( std::size( asset.images ) );
    std::ranges::for_each( asset.images, [ & ]( const auto& image ) {
        std::optional< ve::Image > loadedImage{ loadImage( asset, scene, image ) };
        if ( loadedImage.has_value() ) {
            scene.images.emplace_back( std::move( loadedImage.value() ) );
        }
    } );
}

std::optional< fastgltf::Asset > Loader::getAsset( const std::filesystem::path& path ) {
    constexpr auto loadingOptions{ fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
                                   fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers };

    fastgltf::GltfDataBuffer data;
    auto expectedData{ fastgltf::GltfDataBuffer::FromPath( path ) };
    if ( expectedData.error() != fastgltf::Error::None )
        spdlog::error( "Failed to obtain gltf data buffer: {}", path.string() );

    data = std::move( expectedData.get() );

    const auto objectType{ fastgltf::determineGltfFileType( data ) };
    switch ( objectType ) {

    case fastgltf::GltfType::glTF: {
        auto loadedAsset{ m_parser.loadGltf( data, path.parent_path(), loadingOptions ) };
        if ( loadedAsset.error() == fastgltf::Error::None )
            return std::move( loadedAsset.get() );
        break;
    }

    case fastgltf::GltfType::GLB: {
        auto loadedAsset{ m_parser.loadGltfBinary( data, path.parent_path(), loadingOptions ) };
        if ( loadedAsset.error() == fastgltf::Error::None )
            return std::move( loadedAsset.get() );
        break;
    }

    default: {
        spdlog::error( "Failed to determine gltf file type for path: {}", path.string() );
        return std::nullopt;
    }
    }

    spdlog::error( "Failed to load model: {}", path.string() );
    return std::nullopt;
}

Loader::MaterialsOpt Loader::loadMeterials( const fastgltf::Asset& asset, ve::gltf::Scene& scene ) {
    const std::uint64_t bufferSize{ sizeof( Constants ) * ve::utils::size( asset.materials ) };
    if ( bufferSize == 0U ) {
        spdlog::info( "Asset <{}> does not contain any materials", scene.path.filename().string() );
        return std::nullopt;
    }

    loadImages( asset, scene );

    std::vector< ve::gltf::Material * > tempMaterials;
    scene.materialDataBuffer.emplace( m_memoryAllocator, bufferSize );

    Constants *mappedConstanst{ static_cast< Constants * >( scene.materialDataBuffer->getMappedMemory() ) };
    size_t index{};
    std::string materialName{};
    std::ranges::for_each( asset.materials, [ this, &index, &asset, &scene, &tempMaterials, mappedConstanst,
                                              &materialName ]( const fastgltf::Material& material ) {
        mappedConstanst[ index ] = loadConstanst( material );
        const auto materialType{ material.alphaMode == fastgltf::AlphaMode::Blend ? ve::Material::Type::eTransparent
                                                                                  : ve::Material::Type::eMainColor };
        const auto resources{ loadResources( index, scene, asset, material ) };

        auto& materialBuilder{ m_engine.getMaterialBuiler() };
        materialName =
            material.name.empty() ? std::format( "material{}", std::size( scene.materials ) ) : material.name.c_str();
        const auto& materialPair{ scene.materials.emplace(
            materialName,
            materialBuilder.writeMaterial( materialType, resources, scene.descriptorAllocator.value() ) ) };

        ve::gltf::Material *tempMaterial{ &materialPair.first->second };
        tempMaterials.emplace_back( tempMaterial );

        index++;
    } );

    return tempMaterials;
}

std::vector< ve::MeshAsset * > Loader::loadMeshes( const fastgltf::Asset& asset, ve::gltf::Scene& scene ) {
    const auto materials{ loadMeterials( asset, scene ) };

    std::vector< uint32_t > indices;
    std::vector< ve::Vertex > vertices;

    std::vector< ve::MeshAsset * > tempMeshes;
    tempMeshes.reserve( std::size( asset.meshes ) );

    std::string meshName{};
    std::ranges::for_each( asset.meshes, [ & ]( const auto& mesh ) {
        meshName = mesh.name.empty() ? std::format( "mesh{}", std::size( scene.meshes ) ) : mesh.name.c_str();
        ve::MeshAsset& newMesh{ scene.meshes.emplace( meshName, ve::MeshAsset{} ).first->second };
        tempMeshes.emplace_back( &newMesh );

        indices.clear();
        vertices.clear();

        std::ranges::for_each( mesh.primitives, [ this, &indices, &vertices, &asset, &materials,
                                                  &newMesh ]( const auto& primitive ) {
            ve::Surface surface;
            surface.startIndex = ve::utils::size( indices );
            surface.count = static_cast< uint32_t >( asset.accessors.at( primitive.indicesAccessor.value() ).count );

            size_t initialIndex{ std::size( vertices ) };
            loadIndices( initialIndex, indices, asset, primitive );
            loadVertices( initialIndex, vertices, asset, primitive );
            loadNormals( initialIndex, vertices, asset, primitive );
            loadTextureCoord( initialIndex, vertices, asset, primitive );
            loadColor( initialIndex, vertices, asset, primitive );
            loadTangent( initialIndex, vertices, asset, primitive );

            if ( primitive.materialIndex.has_value() && materials.has_value() ) {
                surface.material.emplace( *materials->at( primitive.materialIndex.value() ) );
            } else {
                surface.material.emplace( m_engine.getDefaultMaterial() );
            }

            newMesh.surfaces.emplace_back( surface );
        } );

        newMesh.buffers = m_engine.uploadMeshBuffers( vertices, indices );
        newMesh.name    = meshName;
    } );

    return tempMeshes;
}

std::vector< std::shared_ptr< ve::Node > > Loader::loadNodes( const fastgltf::Asset& asset, ve::gltf::Scene& scene ) {
    const auto meshes{ loadMeshes( asset, scene ) };
    std::vector< std::shared_ptr< ve::Node > > tempNodes;
    tempNodes.reserve( std::size( asset.nodes ) );

    std::string nodeName{};
    std::ranges::for_each( asset.nodes, [ &meshes, &tempNodes, &scene, &nodeName ]( const fastgltf::Node& node ) {
        std::shared_ptr< ve::Node > newNode;
        if ( node.meshIndex.has_value() ) {
            const ve::MeshAsset& asset{ *meshes.at( node.meshIndex.value() ) };
            newNode = std::make_shared< ve::MeshNode >( asset );
        } else {
            newNode = std::make_shared< ve::Node >();
        }

        tempNodes.emplace_back( newNode );

        const auto matrix{ [ &newNode ]( const fastgltf::math::fmat4x4& matrix ) {
            glm::mat4& localTransform{ newNode->getLocalTransform() };
            memcpy( &localTransform, std::data( matrix ), sizeof( matrix ) );
        } };

        const auto transform{ [ &newNode ]( const fastgltf::TRS& transform ) {
            const glm::vec3 translation( transform.translation[ 0 ], transform.translation[ 1 ],
                                         transform.translation[ 2 ] );
            const glm::quat rotation( transform.rotation[ 3 ], transform.rotation[ 0 ], transform.rotation[ 1 ],
                                      transform.rotation[ 2 ] );
            const glm::vec3 scale( transform.scale[ 0 ], transform.scale[ 1 ], transform.scale[ 2 ] );

            const glm::mat4 translationMat{ glm::translate( glm::mat4( 1.f ), translation ) };
            const glm::mat4 rotationMat{ glm::toMat4( rotation ) };
            const glm::mat4 scaleMat{ glm::scale( glm::mat4( 1.f ), scale ) };

            newNode->setLocalTransform( translationMat * rotationMat * scaleMat );
        } };

        const fastgltf::visitor visitor{ matrix, transform };
        std::visit( visitor, node.transform );

        nodeName = node.name.empty() ? std::format( "node{}", std::size( scene.nodes ) ) : node.name.c_str();
        scene.nodes.emplace( nodeName, newNode );
    } );

    return tempNodes;
}

void Loader::setNodesRalationship( const fastgltf::Asset& asset, ve::gltf::Scene& scene ) {
    const auto tempNodes{ loadNodes( asset, scene ) };

    for ( size_t index{ 0 }; index < std::size( asset.nodes ); index++ ) {
        const fastgltf::Node& node{ asset.nodes.at( index ) };
        std::shared_ptr< Node > sceneNode{ tempNodes.at( index ) };

        std::ranges::for_each( node.children, [ &sceneNode, &tempNodes ]( const auto& childNodeIndex ) {
            sceneNode->addChild( tempNodes.at( childNodeIndex ) );
            tempNodes.at( childNodeIndex )->setParent( sceneNode );
        } );
    }

    std::ranges::for_each( tempNodes, [ &scene ]( auto& node ) {
        if ( node->getParent().lock() == nullptr ) {
            scene.topNodes.emplace_back( node );
            node->refreshWorldTransform( glm::mat4{ 1.0F } );
        }
    } );
}

Loader::Constants Loader::loadConstanst( const fastgltf::Material& material ) {
    Constants constanst;
    constanst.colorFactors.x = material.pbrData.baseColorFactor[ 0 ];
    constanst.colorFactors.y = material.pbrData.baseColorFactor[ 1 ];
    constanst.colorFactors.z = material.pbrData.baseColorFactor[ 2 ];
    constanst.colorFactors.w = material.pbrData.baseColorFactor[ 3 ];

    constanst.metalicRoughnessFactors.x = material.pbrData.metallicFactor;
    constanst.metalicRoughnessFactors.y = material.pbrData.roughnessFactor;

    return constanst;
}

Loader::Resources Loader::loadResources( const size_t index, ve::gltf::Scene& scene, const fastgltf::Asset& asset,
                                         const fastgltf::Material& material ) {
    const auto defaultImageView{ m_engine.getDefaultImage().getImageView() };
    const auto defaultSampler{ m_engine.getDefaultSampler().get() };
    const auto uniformBufferOffset{ index * sizeof( Constants ) };

    Resources resources;
    resources.metalicRoughnessImageView = defaultImageView;
    resources.metalicRoughnessSampler   = defaultSampler;
    resources.dataBuffer                = scene.materialDataBuffer->get();
    resources.dataBufferOffset          = uniformBufferOffset;

    const auto getImageViewAndSampler{ [ & ]( const auto& textureInfo ) -> std::pair< vk::ImageView, vk::Sampler > {
        if ( textureInfo.has_value() ) {
            const size_t textureIndex{ textureInfo->textureIndex };
            const size_t imageIndex{ asset.textures.at( textureIndex ).imageIndex.value() };
            const auto samplerIndexOpt{ asset.textures[ textureIndex ].samplerIndex };

            return { scene.images.at( imageIndex ).getImageView(),
                     samplerIndexOpt.has_value() ? scene.samplers.at( samplerIndexOpt.value() ).get()
                                                 : defaultSampler };
        }

        return { defaultImageView, defaultSampler };
    } };

    const auto [ baseColorView, colorSampler ]{ getImageViewAndSampler( material.pbrData.baseColorTexture ) };
    const auto [ normalView, normalSampler ]{ getImageViewAndSampler( material.normalTexture ) };
    const auto [ metallicRoughnessView,
                 metallicRoughnessSampler ]{ getImageViewAndSampler( material.pbrData.metallicRoughnessTexture ) };

    resources.colorImageView            = baseColorView;
    resources.normalMapView             = normalView;
    resources.metalicRoughnessImageView = metallicRoughnessView;
    resources.colorSampler              = colorSampler;
    resources.normalSampler             = normalSampler;
    resources.metalicRoughnessSampler   = metallicRoughnessSampler;

    return resources;
}

void Loader::loadIndices( const size_t initialIndex, std::vector< uint32_t >& indices, const fastgltf::Asset& asset,
                          const fastgltf::Primitive& primitive ) {
    const fastgltf::Accessor& indexAccessor{ asset.accessors.at( primitive.indicesAccessor.value() ) };
    indices.reserve( indices.size() + indexAccessor.count );

    fastgltf::iterateAccessor< uint32_t >( asset, indexAccessor, [ &indices, initialIndex ]( const uint32_t index ) {
        indices.push_back( index + initialIndex );
    } );
}

void Loader::loadVertices( const size_t initialIndex, std::vector< ve::Vertex >& vertices, const fastgltf::Asset& asset,
                           const fastgltf::Primitive& primitive ) {
    const auto accessorIndex{ primitive.findAttribute( "POSITION" )->accessorIndex };
    const fastgltf::Accessor& positionAccessor{ asset.accessors.at( accessorIndex ) };
    vertices.resize( std::size( vertices ) + positionAccessor.count );

    fastgltf::iterateAccessorWithIndex< glm::vec3 >(
        asset, positionAccessor, [ initialIndex, &vertices ]( const glm::vec3 position, const size_t index ) {
            Vertex vertex{};
            vertex.position                     = position;
            vertices.at( initialIndex + index ) = vertex;
        } );
}

void Loader::loadNormals( const size_t initialIndex, std::vector< ve::Vertex >& vertices, const fastgltf::Asset& asset,
                          const fastgltf::Primitive& primitive ) {
    const auto normals{ primitive.findAttribute( "NORMAL" ) };

    if ( normals != std::end( primitive.attributes ) ) {
        const auto accessorNormalIndex{ ( *normals ).accessorIndex };
        const auto& normalAccessor{ asset.accessors.at( accessorNormalIndex ) };

        fastgltf::iterateAccessorWithIndex< glm::vec3 >(
            asset, normalAccessor, [ &initialIndex, &vertices ]( const glm::vec3 normal, const size_t index ) {
                vertices.at( initialIndex + index ).normal = normal;
            } );
    }
}

void Loader::loadTextureCoord( const size_t initialIndex, std::vector< ve::Vertex >& vertices,
                               const fastgltf::Asset& asset, const fastgltf::Primitive& primitive ) {
    const auto uv{ primitive.findAttribute( "TEXCOORD_0" ) };
    if ( uv != std::end( primitive.attributes ) ) {
        const auto accessorUVIndex{ ( *uv ).accessorIndex };
        const auto& uvAccessor{ asset.accessors.at( accessorUVIndex ) };
        fastgltf::iterateAccessorWithIndex< glm::vec2 >(
            asset, uvAccessor, [ &initialIndex, &vertices ]( const glm::vec2 uv, const size_t index ) {
                vertices.at( initialIndex + index ).uv_x = uv.x;
                vertices.at( initialIndex + index ).uv_y = uv.y;
            } );
    }
}

void Loader::loadColor( const size_t initialIndex, std::vector< ve::Vertex >& vertices, const fastgltf::Asset& asset,
                        const fastgltf::Primitive& primitive ) {
    const auto colors{ primitive.findAttribute( "COLOR_0" ) };
    if ( colors != std::end( primitive.attributes ) ) {
        const auto accessorColorIndex{ ( *colors ).accessorIndex };
        const auto& colorAccessor{ asset.accessors.at( accessorColorIndex ) };
        fastgltf::iterateAccessorWithIndex< glm::vec4 >(
            asset, colorAccessor, [ &initialIndex, &vertices ]( const glm::vec4 color, const size_t index ) {
                vertices.at( initialIndex + index ).color = color;
            } );
    }
}

void Loader::loadTangent( const size_t initialIndex, std::vector< ve::Vertex >& vertices, const fastgltf::Asset& asset,
                          const fastgltf::Primitive& primitive ) {
    const auto tangent{ primitive.findAttribute( "TANGENT" ) };
    if ( tangent != std::end( primitive.attributes ) ) {
        const auto accessorTangentIndex{ ( *tangent ).accessorIndex };
        const auto& tagentAccessor{ asset.accessors.at( accessorTangentIndex ) };
        fastgltf::iterateAccessorWithIndex< glm::vec4 >(
            asset, tagentAccessor, [ &initialIndex, &vertices ]( const glm::vec4 tangent, const size_t index ) {
                vertices.at( initialIndex + index ).tangent = tangent;
            } );
    }
}

} // namespace ve::gltf
