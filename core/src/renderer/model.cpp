#include "renderer/model.h"
#include <assimp/postprocess.h>

namespace core
{
void Model::LoadFromNode(const aiScene* scene, const aiNode* node)
{
    for(unsigned i = 0; i < node->mNumMeshes; i++)
    {
        LoadMesh(scene->mMeshes[node->mMeshes[i]]);
    }
    for(unsigned i = 0; i < node->mNumChildren; i++)
    {
        LoadFromNode(scene, node->mChildren[i]);
    }
}

void Model::LoadMesh(const aiMesh* aiMesh)
{
    refactor::Mesh mesh;
    mesh.name = aiMesh->mName.C_Str();
    mesh.positions.reserve(aiMesh->mNumVertices);
    mesh.normals.reserve(aiMesh->mNumVertices);
    mesh.texCoords.reserve(aiMesh->mNumVertices);

    for(unsigned i = 0; i < aiMesh->mNumVertices; i++)
    {
        const auto v = aiMesh->mVertices[i];
        mesh.positions.emplace_back(v.x, v.y, v.z);

        const auto texCoords = aiMesh->mTextureCoords[i];
        mesh.texCoords.emplace_back(texCoords->x, texCoords->y, texCoords->z);

        const auto normal = aiMesh->mNormals[i];
        mesh.normals.emplace_back(normal.x, normal.y, normal.z);

        const auto tangent = aiMesh->mTangents[i];
        mesh.tangents.emplace_back(tangent.x, tangent.y, tangent.z);

        const auto bitangent = aiMesh->mBitangents[i];
        mesh.tangents.emplace_back(bitangent.x, bitangent.y, bitangent.z);
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

ModelIndex ModelManager::ImportModel(std::string_view modelPath)
{
    const auto* scene = importer_.ReadFile(modelPath.data(), aiProcess_CalcTangentSpace | aiProcess_Triangulate);
    if(scene == nullptr)
    {
        LogError(fmt::format("Could not import scene, with error: {}", importer_.GetErrorString()));
        return INVALID_MODEL_INDEX;
    }
    return ImportScene(scene);

}

ModelIndex ModelManager::ImportScene(const aiScene* scene)
{
    const ModelIndex index = {models_.size()};
    Model model;
    model.LoadFromNode(scene, scene->mRootNode);
    models_.push_back(model);
    return index;
}
} // namespace core
