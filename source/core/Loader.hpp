#pragma once

#include "Buffer.hpp"
#include "MemoryAllocator.hpp"
#include "Mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>

#include <vector>
#include <optional>
#include <filesystem>
#include <iostream>

struct aiNode;
struct aiScene;

namespace ve {

class Engine;

class Loader : public utils::NonCopyable,
               public utils::NonMovable {
public:
    Loader( const ve::Engine& engine, const ve::MemoryAllocator& allocator );
    std::vector< MeshAsset > loadMeshes( const std::filesystem::path& path );

private:
    Assimp::Importer m_importer;
    const ve::Engine& m_engine;
    const ve::MemoryAllocator& m_memoryAllocator;

    void processNode( aiNode *const node, const aiScene *scene, std::vector< MeshAsset >& meshAssets );
    MeshAsset processMesh( aiMesh *const mesh );
};

} // namespace ve
