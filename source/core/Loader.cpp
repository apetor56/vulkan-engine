#include "Loader.hpp"
#include "Engine.hpp"

#include <fastgltf/util.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

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

    scene->descriptorAllocator.emplace( logicalDevice, setsCount, sizes );

    scene->samplers.reserve( std::size( asset->samplers ) );
    std::ranges::for_each( asset.value().samplers, [ &scene, &logicalDevice ]( const auto& gltfSampler ) {
        scene->samplers.emplace_back( logicalDevice, gltfSampler );
    } );

    setNodesRalationship( asset.value(), *scene );

    return scene;
}

std::vector< vk::ImageView > Loader::loadImages( const fastgltf::Asset& asset ) {
    std::vector< vk::ImageView > images( std::size( asset.images ) );
    std::ranges::for_each( asset.images, [ this, &images ]( const auto& image ) {
        images.emplace_back( m_engine.getDefaultImage().getImageView() );
    } );

    return images;
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

    default:
        spdlog::error( "Failed to determine gltf file type for path: {}", path.string() );
    }

    spdlog::error( "Failed to load model: {}", path.string() );
    return std::nullopt;
}

std::vector< ve::gltf::Material * > Loader::loadMeterials( const fastgltf::Asset& asset, ve::gltf::Scene& scene ) {
    const auto images{ loadImages( asset ) };
    std::vector< ve::gltf::Material * > tempMaterials;

    const std::uint64_t bufferSize{ sizeof( Constants ) * ve::utils::size( asset.materials ) };
    scene.materialDataBuffer.emplace( m_memoryAllocator, bufferSize );

    Constants *mappedConstanst{ static_cast< Constants * >( scene.materialDataBuffer->getMappedMemory() ) };
    size_t index{};
    std::ranges::for_each( asset.materials, [ this, &index, &asset, &scene, &images, &tempMaterials,
                                              mappedConstanst ]( const fastgltf::Material& material ) {
        mappedConstanst[ index ] = loadConstanst( material );
        const auto materialType{ material.alphaMode == fastgltf::AlphaMode::Blend ? ve::Material::Type::eTransparent
                                                                                  : ve::Material::Type::eMainColor };
        const auto resources{ loadResources( index, scene, asset, material, images ) };

        auto& materialBuilder{ m_engine.getMaterialBuiler() };
        const auto& materialPair{ scene.materials.emplace(
            material.name.c_str(),
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

    std::ranges::for_each( asset.meshes, [ & ]( const auto& mesh ) {
        ve::MeshAsset& newMesh{ scene.meshes.emplace( mesh.name.c_str(), ve::MeshAsset{} ).first->second };
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

            if ( primitive.materialIndex.has_value() ) {
                surface.material.emplace( *materials.at( primitive.materialIndex.value() ) );
            } else {
                surface.material.emplace( *materials.at( 0 ) );
            }

            newMesh.surfaces.emplace_back( surface );
        } );

        newMesh.buffers = m_engine.uploadMeshBuffers( vertices, indices );
    } );

    return tempMeshes;
}

std::vector< std::shared_ptr< ve::Node > > Loader::loadNodes( const fastgltf::Asset& asset, ve::gltf::Scene& scene ) {
    const auto meshes{ loadMeshes( asset, scene ) };
    std::vector< std::shared_ptr< ve::Node > > tempNodes;

    std::ranges::for_each( asset.nodes, [ &meshes, &tempNodes, &scene ]( const fastgltf::Node& node ) {
        std::shared_ptr< ve::Node > newNode;
        if ( node.meshIndex.has_value() ) {
            const ve::MeshAsset& asset{ *meshes.at( node.meshIndex.value() ) };
            newNode = std::make_shared< ve::MeshNode >( asset );
        } else {
            newNode = std::make_shared< ve::Node >();
        }

        tempNodes.emplace_back( newNode );
        // scene.nodes.at( node.name.c_str() );

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
                                         const fastgltf::Material& material, std::span< const vk::ImageView > images ) {
    auto defaultImageView{ m_engine.getDefaultImage().getImageView() };
    auto defaultSampler{ m_engine.getDefaultSampler().get() };
    const auto uniformBufferOffset{ index * sizeof( Constants ) };

    Resources resources;
    resources.metalicRoughnessImageView = defaultImageView;
    resources.metalicRoughnessSampler   = defaultSampler;
    resources.dataBuffer                = scene.materialDataBuffer->get();
    resources.dataBufferOffset          = uniformBufferOffset;

    if ( material.pbrData.baseColorTexture.has_value() ) {
        const size_t textureIndex{ material.pbrData.baseColorTexture.value().textureIndex };
        const size_t imageIndex{ asset.textures[ textureIndex ].imageIndex.value() };
        const size_t samplerIndex{ asset.textures[ textureIndex ].samplerIndex.value() };

        resources.colorImageView = defaultImageView;
        // resources.colorImageView = images[ textureIndex ];
        resources.colorSampler = scene.samplers.at( samplerIndex ).get();
    } else {
        resources.colorImageView = defaultImageView;
        resources.colorSampler   = defaultSampler;
    }

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
                vertices[ initialIndex + index ].uv_x = uv.x;
                vertices[ initialIndex + index ].uv_y = uv.y;
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
                vertices[ initialIndex + index ].color = color;
            } );
    }
}

} // namespace ve::gltf
