#include "Node.hpp"

namespace ve {

void Node::render( const glm::mat4& topMatrix, RenderContext& renderContext ) {
    std::ranges::for_each( m_children, [ & ]( auto& child ) { child->render( topMatrix, renderContext ); } );
}

void Node::refreshWorldTransform( const glm::mat4& parentMatrix ) {
    m_worldTransform = parentMatrix * m_localTransform;
    std::ranges::for_each( m_children, [ this ]( auto& child ) { child->refreshWorldTransform( m_worldTransform ); } );
}

void MeshNode::render( const glm::mat4& topMatrix, RenderContext& renderContext ) {
    const glm::mat4 nodeMatrix{ topMatrix * m_worldTransform };
    const auto& surface{ m_asset.surface };
    const auto& buffers{ m_asset.buffers };

    renderContext.opaqueSurfaces.emplace_back( nodeMatrix, buffers.indexBuffer->get(), surface.material->data,
                                               buffers.vertexBufferAddress, surface.count, surface.startIndex );

    Node::render( topMatrix, renderContext );
}

} // namespace ve
