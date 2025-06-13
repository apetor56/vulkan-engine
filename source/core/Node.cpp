#include "Node.hpp"

#include <spdlog/spdlog.h>

namespace ve {

void Node::render( const glm::mat4& topMatrix, RenderContext& renderContext ) {
    std::ranges::for_each( m_children, [ & ]( auto& child ) { child->render( topMatrix, renderContext ); } );
}

void Node::refreshWorldTransform( const glm::mat4& parentMatrix ) {
    m_worldTransform = parentMatrix * m_localTransform;
    std::ranges::for_each( m_children, [ this ]( auto& child ) { child->refreshWorldTransform( m_worldTransform ); } );
}

void Node::setLocalTransform( const glm::mat4& transform ) {
    m_localTransform = transform;
}

void Node::addChild( std::shared_ptr< Node > child ) {
    m_children.emplace_back( child );
}

void Node::setParent( std::shared_ptr< Node > parent ) {
    m_parent = parent;
}

void MeshNode::render( const glm::mat4& topMatrix, RenderContext& renderContext ) {
    const glm::mat4 nodeMatrix{ topMatrix * m_worldTransform };
    const auto& buffers{ m_asset.buffers };

    std::ranges::for_each( m_asset.surfaces, [ &buffers, &renderContext, &nodeMatrix ]( const auto& surface ) {
        switch ( surface.material->data.type ) {
        case ve::Material::Type::eMainColor: {
            renderContext.opaqueSurfaces.emplace_back( nodeMatrix, buffers.indexBuffer->get(), surface.material->data,
                                                       buffers.vertexBufferAddress, surface.count, surface.startIndex );
            break;
        }

        case ve::Material::Type::eTransparent: {
            renderContext.transparentSurfaces.emplace_back( nodeMatrix, buffers.indexBuffer->get(),
                                                            surface.material->data, buffers.vertexBufferAddress,
                                                            surface.count, surface.startIndex );
            break;
        }

        default: {
            spdlog::warn( "Material type with value {} not handled.",
                          static_cast< int >( surface.material->data.type ) );
        }
        }
    } );

    Node::render( topMatrix, renderContext );
}

} // namespace ve

namespace ve::gltf {

void Scene::render( const glm::mat4& topMatrix, ve::RenderContext& renderContext ) {
    std::ranges::for_each( topNodes, [ & ]( auto& topNode ) { topNode->render( topMatrix, renderContext ); } );
}

} // namespace ve::gltf
