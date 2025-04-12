#pragma once

#include "Material.hpp"
#include "Mesh.hpp"

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

protected:
    glm::mat4 m_localTransform{ 1.0F };
    glm::mat4 m_worldTransform{ 1.0F };
    std::vector< std::shared_ptr< Node > > m_children;
    std::weak_ptr< Node > m_parent;
};

class MeshNode : public Node {
public:
    MeshNode( const MeshAsset& meshAsset ) : m_asset{ meshAsset } {}

    virtual void render( const glm::mat4& topMatrix, RenderContext& renderContext ) override;

private:
    const MeshAsset& m_asset;
};

} // namespace ve
