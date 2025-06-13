#pragma once

#include "Material.hpp"
#include "Mesh.hpp"
#include "Sampler.hpp"

#include "descriptor/DescriptorAllocator.hpp"

namespace ve {

struct RenderObject {
    const glm::mat4 transform;
    const vk::Buffer indexBuffer;
    const ve::Material& material;
    const vk::DeviceAddress vertexBufferAddress;
    const uint32_t indexCount{};
    const uint32_t firstIndex{};
};

struct RenderContext {
    std::vector< RenderObject > opaqueSurfaces;
    std::vector< RenderObject > transparentSurfaces;
};

class Renderable {
public:
    virtual ~Renderable()                                                           = default;
    virtual void render( const glm::mat4& topMatrix, RenderContext& renderContext ) = 0;
};

class Node : public Renderable {
public:
    virtual void render( const glm::mat4& topMatrix, RenderContext& renderContext ) override;
    void refreshWorldTransform( const glm::mat4& parentMatrix );

    void setLocalTransform( const glm::mat4& transform );
    void addChild( std::shared_ptr< Node > child );
    void setParent( std::shared_ptr< Node > parent );

    std::weak_ptr< Node > getParent() const noexcept { return m_parent; }
    glm::mat4& getLocalTransform() noexcept { return m_localTransform; }

protected:
    glm::mat4 m_localTransform{ 1.0F };
    glm::mat4 m_worldTransform{ 1.0F };
    std::vector< std::shared_ptr< Node > > m_children;
    std::weak_ptr< Node > m_parent;
};

class MeshNode : public Node {
public:
    MeshNode( const ve::MeshAsset& meshAsset ) : m_asset{ meshAsset } {}

    virtual void render( const glm::mat4& topMatrix, RenderContext& renderContext ) override;

private:
    const ve::MeshAsset& m_asset;
};

} // namespace ve

namespace ve::gltf {

struct Scene : public ve::Renderable {
    ~Scene() {}

    using MeshMap     = std::unordered_map< std::string, ve::MeshAsset >;
    using NodeMap     = std::unordered_map< std::string, std::shared_ptr< ve::Node > >;
    using MaterialMap = std::unordered_map< std::string, ve::gltf::Material >;

    virtual void render( const glm::mat4& topMatrix, ve::RenderContext& renderContext ) override;

    MeshMap meshes;
    NodeMap nodes;
    MaterialMap materials;
    std::filesystem::path path;
    std::vector< ve::Image > images;
    std::vector< std::shared_ptr< ve::Node > > topNodes;
    std::vector< ve::Sampler > samplers;
    std::optional< ve::DescriptorAllocator > descriptorAllocator;
    std::optional< ve::UniformBuffer > materialDataBuffer;
};

} // namespace ve::gltf
