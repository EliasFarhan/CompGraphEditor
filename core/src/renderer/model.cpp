#include "renderer/model.h"

#include "engine/filesystem.h"
#include "utils/log.h"

#include <assimp/postprocess.h>
#include <fmt/format.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace core
{

const Mesh& Model::GetMesh(std::string_view meshName)
{
    const auto it = std::ranges::find_if(meshes_, [&meshName](const auto& mesh)
    {
        return meshName == mesh.name;
    });
    return *it;
}

void Model::LoadFromNode(const aiScene* scene, const aiNode* node)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for(unsigned i = 0; i < node->mNumMeshes; i++)
    {
        LoadMesh(scene->mMeshes[node->mMeshes[i]]);
    }
    for(unsigned i = 0; i < node->mNumChildren; i++)
    {
        LoadFromNode(scene, node->mChildren[i]);
    }
}

void Model::LoadMaterials(const aiScene* scene)
{
    materials_.reserve(scene->mNumMaterials);
    for(unsigned i = 0; i < scene->mNumMaterials; i++)
    {
        const auto* material = scene->mMaterials[i];
        core::ModelMaterial newMaterial = { material->GetName().C_Str() };
        for(unsigned j = 0; j < core::pb::TextureType::LENGTH; j++)
        {
            const auto textureType = static_cast<aiTextureType>(j);
            aiString textureName;
            if(material->GetTextureCount(textureType) > 0)
            {

                const auto returnStatus = material->GetTexture(textureType, 0, &textureName);
                if (returnStatus == aiReturn_SUCCESS)
                {
                    newMaterial.textures[j] = textureName.C_Str();
                }
                else
                {
                    LogError(fmt::format("Error whole retrieving texture type {} from scene {}", aiTextureTypeToString(textureType), scene->mName.C_Str()));
                }
            }
        }
        materials_.push_back(newMaterial);
    }
}

void Model::LoadMesh(const aiMesh* aiMesh)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    Mesh mesh;
    mesh.name = aiMesh->mName.C_Str();
    mesh.materialIndex = aiMesh->mMaterialIndex;
    mesh.vertices.reserve(aiMesh->mNumVertices);

    for(unsigned i = 0; i < aiMesh->mNumVertices; i++)
    {
        Vertex vertex{};
        const auto& v = aiMesh->mVertices[i];
        vertex.position = { v.x, v.y, v.z };

        const auto& texCoords = aiMesh->mTextureCoords[0][i];
        vertex.texCoords = { texCoords.x, texCoords.y };

        const auto& normal = aiMesh->mNormals[i];
        vertex.normal = { normal.x, normal.y, normal.z };

        const auto& tangent = aiMesh->mTangents[i];
        vertex.tangent = { tangent.x, tangent.y, tangent.z };

        const auto& bitangent = aiMesh->mBitangents[i];
        vertex.bitangent = { bitangent.x, bitangent.y, bitangent.z };

        mesh.vertices.push_back(vertex);
    }

    mesh.indices.reserve(aiMesh->mNumFaces * 3u);
    for(unsigned i = 0; i < aiMesh->mNumFaces; i++)
    {
        const auto& face = aiMesh->mFaces[i];
        for(unsigned j = 0; j < face.mNumIndices; j++)
        {
            mesh.indices.push_back(face.mIndices[j]);
        }
    }
    meshes_.push_back(mesh);

}

ModelManager::ModelManager()
{
    importer_.SetIOHandler(new IOSystem());
}

ModelIndex ModelManager::ImportModel(const core::Path &modelPath)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    const auto it = modelNamesMap_.find(modelPath.c_str());
    if(it != modelNamesMap_.end())
    {
        return it->second;
    }

    const auto& filesystem = core::FilesystemLocator::get();
    const auto exists = filesystem.FileExists(Path(modelPath));
    if (!exists)
    {
        LogError(fmt::format("Could not find: {}", modelPath));
        return INVALID_MODEL_INDEX;
    }
    const auto* scene = importer_.ReadFile(modelPath.c_str(),
        aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs);
    
    if(scene == nullptr)
    {
        LogError(fmt::format("Could not import scene, with error: {}", importer_.GetErrorString()));
        return INVALID_MODEL_INDEX;
    }
    const auto modelIndex = ImportScene(scene);
    modelNamesMap_[modelPath.c_str()] = modelIndex;
    return modelIndex;

}

void ModelManager::Clear()
{
    models_.clear();
}

ModelIndex ModelManager::ImportScene(const aiScene* scene)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const ModelIndex index = {models_.size()};
    Model model;
    model.LoadMaterials(scene);
    model.LoadFromNode(scene, scene->mRootNode);
    models_.push_back(model);
    return index;
}
} // namespace core
