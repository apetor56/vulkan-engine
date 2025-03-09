#include "Loader.hpp"
#include "Engine.hpp"

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <exception>

namespace ve {

Loader::Loader( const ve::Engine& engine, const ve::MemoryAllocator& allocator )
    : m_engine{ engine }, m_memoryAllocator{ allocator } {}

std::vector< MeshAsset > Loader::loadMeshes( const std::filesystem::path& path ) {
    const aiScene *scene{ m_importer.ReadFile( path.string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs ) };
    if ( scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr )
        throw std::runtime_error( "failed to load meshes" );

    std::vector< MeshAsset > meshAssets;
    meshAssets.reserve( scene->mNumMeshes );
    processNode( scene->mRootNode, scene, meshAssets );

    return meshAssets;
}

void Loader::processNode( aiNode *const node, const aiScene *scene, std::vector< MeshAsset >& meshAssets ) {
    for ( uint32_t meshArrayIndex{}; meshArrayIndex < node->mNumMeshes; meshArrayIndex++ ) {
        const auto meshIndex{ node->mMeshes[ meshArrayIndex ] };
        aiMesh *const mesh{ scene->mMeshes[ meshIndex ] };
        meshAssets.emplace_back( processMesh( mesh ) );
    }

    for ( uint32_t childIndex{}; childIndex < node->mNumChildren; childIndex++ )
        processNode( node->mChildren[ childIndex ], scene, meshAssets );
}

MeshAsset Loader::processMesh( aiMesh *const mesh ) {
    std::vector< Vertex > vertices;
    vertices.reserve( mesh->mNumVertices );
    std::vector< uint32_t > indices;
    indices.reserve( mesh->mNumVertices * 2 );

    Vertex vertex{ .color{ 0.5F, 0.5F, 0.5F } };
    aiVector3D meshVertex{};
    aiVector3D meshTexCoord{};
    for ( uint32_t vertexID{}; vertexID < mesh->mNumVertices; vertexID++ ) {
        meshVertex      = mesh->mVertices[ vertexID ];
        vertex.position = { meshVertex.x, meshVertex.y, meshVertex.z };

        if ( mesh->mTextureCoords[ 0 ] != nullptr ) {
            meshTexCoord    = mesh->mTextureCoords[ 0 ][ vertexID ];
            vertex.texCoord = { meshTexCoord.x, meshTexCoord.y };
        } else {
            vertex.texCoord = { 0.0F, 0.0F };
        }

        vertices.emplace_back( vertex );
    }

    aiFace face{};
    for ( uint32_t faceID{}; faceID < mesh->mNumFaces; faceID++ ) {
        face = mesh->mFaces[ faceID ];
        for ( uint32_t indexID{}; indexID < face.mNumIndices; indexID++ )
            indices.emplace_back( face.mIndices[ indexID ] );
    }

    return MeshAsset{ .buffers{ m_engine.uploadMeshBuffers( vertices, indices ) }, .name{ mesh->mName } };
}

} // namespace ve
