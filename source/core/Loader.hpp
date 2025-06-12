#pragma once

#include "Node.hpp"

#include "utils/NonCopyable.hpp"
#include "utils/NonMovable.hpp"

#include <fastgltf/core.hpp>

#include <filesystem>

namespace ve {
class Engine;
class MemoryAllocator;
} // namespace ve

namespace fastgltf {
class Asset;
}

namespace ve::gltf {

class Loader : public utils::NonCopyable,
               public utils::NonMovable {
public:
    Loader( ve::Engine& engine, const ve::MemoryAllocator& allocator );

    std::optional< std::shared_ptr< ve::gltf::Scene > > load( const std::filesystem::path& path );

private:
    using Constants = ve::gltf::MetalicRoughness::Constants;
    using Resources = ve::gltf::MetalicRoughness::Resources;

    fastgltf::Parser m_parser{};
    ve::Engine& m_engine;
    const ve::MemoryAllocator& m_memoryAllocator;

    std::optional< fastgltf::Asset > getAsset( const std::filesystem::path& path );
    std::vector< vk::ImageView > loadImages( const fastgltf::Asset& asset );
    std::vector< ve::gltf::Material * > loadMeterials( const fastgltf::Asset& asset, ve::gltf::Scene& scene );
    std::vector< ve::MeshAsset * > loadMeshes( const fastgltf::Asset& asset, ve::gltf::Scene& scene );
    std::vector< std::shared_ptr< ve::Node > > loadNodes( const fastgltf::Asset& asset, ve::gltf::Scene& scene );
    void setNodesRalationship( const fastgltf::Asset& asset, ve::gltf::Scene& scene );

    Constants loadConstanst( const fastgltf::Material& material );
    Resources loadResources( const size_t index, ve::gltf::Scene& scene, const fastgltf::Asset& asset,
                             const fastgltf::Material& material, std::span< const vk::ImageView > images );

    void loadIndices( const size_t initialIndex, std::vector< uint32_t >& indices, const fastgltf::Asset& asset,
                      const fastgltf::Primitive& primitive );
    void loadVertices( const size_t initialIndex, std::vector< ve::Vertex >& vertices, const fastgltf::Asset& asset,
                       const fastgltf::Primitive& primitive );
    void loadNormals( const size_t initialIndex, std::vector< ve::Vertex >& vertices, const fastgltf::Asset& asset,
                      const fastgltf::Primitive& primitive );
    void loadTextureCoord( const size_t initialIndex, std::vector< ve::Vertex >& vertices, const fastgltf::Asset& asset,
                           const fastgltf::Primitive& primitive );
    void loadColor( const size_t initialIndex, std::vector< ve::Vertex >& vertices, const fastgltf::Asset& asset,
                    const fastgltf::Primitive& primitive );
};

} // namespace ve::gltf
