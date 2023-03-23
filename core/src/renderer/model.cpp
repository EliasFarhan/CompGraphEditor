#include "renderer/model.h"
#include <assimp/postprocess.h>


#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace core
{
namespace refactor
{
Mesh GenerateQuad(glm::vec3 scale, glm::vec3 offset)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    Mesh mesh{};
    mesh.name = "quad";
    mesh.vertices = {
        {glm::vec3(0.5f, 0.5f, 0.0f) * scale + offset, glm::vec2(1.0f, 1.0f), glm::vec3(0,0,-1)},// top right
        {glm::vec3(0.5f, -0.5f, 0.0f) * scale + offset, glm::vec2(1.0f, 0.0f), glm::vec3(0,0,-1)}, // bottom right
        {glm::vec3(-0.5f, -0.5f, 0.0f) * scale + offset, glm::vec2(0.0f, 0.0f), glm::vec3(0,0,-1)},  // bottom left
        {glm::vec3(-0.5f, 0.5f, 0.0f) * scale + offset, glm::vec2(0.0f, 1.0f), glm::vec3(0,0,-1)},// top left
    };
    
    {
        const glm::vec3 edge1 = mesh.vertices[1].position - mesh.vertices[0].position;
        const glm::vec3 edge2 = mesh.vertices[2].position - mesh.vertices[0].position;
        const glm::vec2 deltaUV1 = mesh.vertices[1].texCoords - mesh.vertices[0].texCoords;
        const glm::vec2 deltaUV2 = mesh.vertices[2].texCoords - mesh.vertices[0].texCoords;

        const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        mesh.vertices[0].tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        mesh.vertices[0].tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        mesh.vertices[0].tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        mesh.vertices[0].bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        mesh.vertices[0].bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        mesh.vertices[0].bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        mesh.vertices[1].tangent = mesh.vertices[0].tangent;
        mesh.vertices[2].tangent = mesh.vertices[0].tangent;

        mesh.vertices[1].bitangent = mesh.vertices[0].bitangent;
        mesh.vertices[2].bitangent = mesh.vertices[0].bitangent;
    }

    mesh.indices = {
        // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    return mesh;
}
Mesh GenerateCube(glm::vec3 scale, glm::vec3 offset)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    Mesh cube{};
    cube.name = "cube";
    cube.indices =
    {
        0,1,2,0,3,1,
        4,5,6,6,7,4,
        8,9,10,9,8,11,
        12,13,14,14,15,12,
        16,17,18,18,19,16,
        20,21,22,21,20,23
    };
    cube.vertices =
    {
        //Right face
        {glm::vec3(0.5f, 0.5f, 0.5f) * scale + offset ,  glm::vec2(1.0f, 0.0f),glm::vec3(1.0f, 0.0f, 0.0f),},//0  
        {glm::vec3(0.5f, -0.5f, -0.5f) * scale + offset ,glm::vec2(0.0f, 1.0f),glm::vec3(1.0f, 0.0f, 0.0f), },//1
        {glm::vec3(0.5f, 0.5f, -0.5f) * scale + offset , glm::vec2(1.0f, 1.0f),glm::vec3(1.0f, 0.0f, 0.0f), },//2
        {glm::vec3(0.5f, -0.5f, 0.5f) * scale + offset , glm::vec2(0.0f, 0.0f),glm::vec3(1.0f, 0.0f, 0.0f), },//3
        //Left face                 
        {glm::vec3(-0.5f, 0.5f, 0.5f) * scale + offset ,  glm::vec2(1.0f, 0.0f),glm::vec3(-1.0f, 0.0f, 0.0f),},//4
        {glm::vec3(-0.5f, 0.5f, -0.5f) * scale + offset , glm::vec2(1.0f, 1.0f),glm::vec3(-1.0f, 0.0f, 0.0f),},//5
        {glm::vec3(-0.5f, -0.5f, -0.5f) * scale + offset ,glm::vec2(0.0f, 1.0f),glm::vec3(-1.0f, 0.0f, 0.0f),}, //6
        {glm::vec3(-0.5f, -0.5f, 0.5f) * scale + offset , glm::vec2(0.0f, 0.0f),glm::vec3(-1.0f, 0.0f, 0.0f),},//7
        //Top face 
        {glm::vec3(-0.5f, 0.5f, -0.5f) * scale + offset ,glm::vec2(0.0f, 1.0f),glm::vec3(0.0f, 1.0f, 0.0f),},//8
        {glm::vec3(0.5f, 0.5f, 0.5f) * scale + offset ,  glm::vec2(1.0f, 0.0f),glm::vec3(0.0f, 1.0f, 0.0f),},//9
        {glm::vec3(0.5f, 0.5f, -0.5f) * scale + offset , glm::vec2(1.0f, 1.0f),glm::vec3(0.0f, 1.0f, 0.0f),},//10
        {glm::vec3(-0.5f, 0.5f, 0.5f) * scale + offset , glm::vec2(0.0f, 0.0f),glm::vec3(0.0f, 1.0f, 0.0f),},//11
        //Bottom face
        {glm::vec3(-0.5f, -0.5f, -0.5f) * scale + offset ,glm::vec2(0.0f, 1.0f),glm::vec3(0.0f, -1.0f, 0.0f),}, //12
        {glm::vec3(0.5f, -0.5f, -0.5f) * scale + offset , glm::vec2(1.0f, 1.0f),glm::vec3(0.0f, -1.0f, 0.0f),},//13
        {glm::vec3(0.5f, -0.5f, 0.5f) * scale + offset ,  glm::vec2(1.0f, 0.0f),glm::vec3(0.0f, -1.0f, 0.0f),},//14
        {glm::vec3(-0.5f, -0.5f, 0.5f) * scale + offset , glm::vec2(0.0f, 0.0f),glm::vec3(0.0f, -1.0f, 0.0f),},//15
        //Front face
        {glm::vec3(-0.5f, -0.5f, 0.5f) * scale + offset ,glm::vec2(0.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),},//16
        {glm::vec3(0.5f, -0.5f, 0.5f) * scale + offset , glm::vec2(1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),},//17
        {glm::vec3(0.5f, 0.5f, 0.5f) * scale + offset ,  glm::vec2(1.0f, 1.0f),glm::vec3(0.0f, 0.0f, 1.0f),},//18
        {glm::vec3(-0.5f, 0.5f, 0.5f) * scale + offset , glm::vec2(0.0f, 1.0f),glm::vec3(0.0f, 0.0f, 1.0f),},//19
        //Back face
        {glm::vec3(-0.5f, -0.5f, -0.5f) * scale + offset ,glm::vec2(0.0f, 0.0f),glm::vec3(0.0f, 0.0f, -1.0f),},//20
        {glm::vec3(0.5f, 0.5f, -0.5f) * scale + offset ,  glm::vec2(1.0f, 1.0f),glm::vec3(0.0f, 0.0f, -1.0f),},//21
        {glm::vec3(0.5f, -0.5f, -0.5f) * scale + offset , glm::vec2(1.0f, 0.0f),glm::vec3(0.0f, 0.0f, -1.0f),},//22
        {glm::vec3(-0.5f, 0.5f, -0.5f) * scale + offset , glm::vec2(0.0f, 1.0f),glm::vec3(0.0f, 0.0f, -1.0f),},//23
    };
    for (int i = 0; i < 36; i += 3)
    {
        std::array triIndices =
        {
            cube.indices[i],
            cube.indices[i + 1],
            cube.indices[i + 2],
        };
        const glm::vec3 edge1 = cube.vertices[triIndices[1]].position - cube.vertices[triIndices[0]].position;
        const glm::vec3 edge2 = cube.vertices[triIndices[2]].position - cube.vertices[triIndices[0]].position;
        const glm::vec2 deltaUV1 = cube.vertices[triIndices[1]].texCoords - cube.vertices[triIndices[0]].texCoords;
        const glm::vec2 deltaUV2 = cube.vertices[triIndices[2]].texCoords - cube.vertices[triIndices[0]].texCoords;

        const float f =
            1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        cube.vertices[triIndices[0]].tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        cube.vertices[triIndices[0]].tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        cube.vertices[triIndices[0]].tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        cube.vertices[triIndices[1]].tangent = cube.vertices[triIndices[0]].tangent;
        cube.vertices[triIndices[2]].tangent = cube.vertices[triIndices[0]].tangent;

        cube.vertices[triIndices[0]].bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        cube.vertices[triIndices[0]].bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        cube.vertices[triIndices[0]].bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        cube.vertices[triIndices[1]].bitangent = cube.vertices[triIndices[0]].bitangent;
        cube.vertices[triIndices[2]].bitangent = cube.vertices[triIndices[0]].bitangent;
    }
    return cube;
}
Mesh GenerateSphere(float scale, glm::vec3 offset)
{
    return {};
}
}

const refactor::Mesh& Model::GetMesh(std::string_view meshName)
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
        core::refactor::Material newMaterial = { material->GetName().C_Str() };
        for(unsigned j = 0; j < core::pb::TextureType::LENGTH; j++)
        {
            const auto textureType = static_cast<aiTextureType>(j);
            aiString textureName;
            if(material->GetTextureCount(textureType)>0)
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
    refactor::Mesh mesh;
    mesh.name = aiMesh->mName.C_Str();
    mesh.materialIndex = aiMesh->mMaterialIndex;
    mesh.vertices.reserve(aiMesh->mNumVertices);

    for(unsigned i = 0; i < aiMesh->mNumVertices; i++)
    {
        refactor::Vertex vertex{};
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

ModelIndex ModelManager::ImportModel(std::string_view modelPath)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    const auto it = modelNamesMap_.find(modelPath.data());
    if(it != modelNamesMap_.end())
    {
        return it->second;
    }

    const auto& filesystem = core::FilesystemLocator::get();
    const auto modelFile = filesystem.LoadFile(modelPath);
    if (modelFile.data == nullptr)
    {
        LogError(fmt::format("Could not open: {}", modelPath));
        return INVALID_MODEL_INDEX;
    }
    const auto* scene = importer_.ReadFileFromMemory(modelFile.data, modelFile.length, 
        aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_GenNormals);
    if(scene == nullptr)
    {
        LogError(fmt::format("Could not import scene, with error: {}", importer_.GetErrorString()));
        return INVALID_MODEL_INDEX;
    }
    const auto modelIndex = ImportScene(scene);
    modelNamesMap_[modelPath.data()] = modelIndex;
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
