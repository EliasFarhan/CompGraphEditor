#include "renderer/model.h"
#include <assimp/postprocess.h>

namespace core
{
namespace refactor
{
Mesh GenerateQuad(glm::vec3 scale, glm::vec3 offset)
{
    Mesh mesh{};
    mesh.name = "quad";
    mesh.positions = {
        glm::vec3(0.5f, 0.5f, 0.0f) * scale + offset,  // top right
        glm::vec3(0.5f, -0.5f, 0.0f) * scale + offset,  // bottom right
        glm::vec3(-0.5f, -0.5f, 0.0f) * scale + offset,  // bottom left
        glm::vec3(-0.5f, 0.5f, 0.0f) * scale + offset  // top left
    };

    mesh.texCoords = {
        glm::vec2(1.0f, 1.0f),      // top right
        glm::vec2(1.0f, 0.0f),   // bottom right
        glm::vec2(0.0f, 0.0f),   // bottom left
        glm::vec2(0.0f, 1.0f),   // bottom left
    };

    mesh.normals = {
        glm::vec3(0,0,-1),
        glm::vec3(0,0,-1),
        glm::vec3(0,0,-1),
        glm::vec3(0,0,-1)
    };

    mesh.tangents.resize(4);
    mesh.bitangents.resize(4);
    {
        const glm::vec3 edge1 = mesh.positions[1] - mesh.positions[0];
        const glm::vec3 edge2 = mesh.positions[2] - mesh.positions[0];
        const glm::vec2 deltaUV1 = mesh.texCoords[1] - mesh.texCoords[0];
        const glm::vec2 deltaUV2 = mesh.texCoords[2] - mesh.texCoords[0];

        const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        mesh.tangents[0].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        mesh.tangents[0].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        mesh.tangents[0].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        mesh.bitangents[0].x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        mesh.bitangents[0].y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        mesh.bitangents[0].z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    }
    std::fill(mesh.tangents.begin() + 1, mesh.tangents.end(), mesh.tangents[0]);
    std::fill(mesh.bitangents.begin() + 1, mesh.bitangents.end(), mesh.bitangents[0]);

    mesh.indices = {
        // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    return mesh;
}
Mesh GenerateCube(glm::vec3 scale, glm::vec3 offset)
{
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
    cube.positions =
    {
        //Right face
        glm::vec3(0.5f, 0.5f, 0.5f) * scale + offset , //0
        glm::vec3(0.5f, -0.5f, -0.5f) * scale + offset , //1
        glm::vec3(0.5f, 0.5f, -0.5f) * scale + offset , //2
        glm::vec3(0.5f, -0.5f, 0.5f) * scale + offset ,//3
        //Left face                 
        glm::vec3(-0.5f, 0.5f, 0.5f) * scale + offset , //4
        glm::vec3(-0.5f, 0.5f, -0.5f) * scale + offset , //5
        glm::vec3(-0.5f, -0.5f, -0.5f) * scale + offset , //6
        glm::vec3(-0.5f, -0.5f, 0.5f) * scale + offset ,//7
        //Top face 
        glm::vec3(-0.5f, 0.5f, -0.5f) * scale + offset ,//8
        glm::vec3(0.5f, 0.5f, 0.5f) * scale + offset ,//9
        glm::vec3(0.5f, 0.5f, -0.5f) * scale + offset ,//10
        glm::vec3(-0.5f, 0.5f, 0.5f) * scale + offset ,//11
        //Bottom face
        glm::vec3(-0.5f, -0.5f, -0.5f) * scale + offset ,//12
        glm::vec3(0.5f, -0.5f, -0.5f) * scale + offset ,//13
        glm::vec3(0.5f, -0.5f, 0.5f) * scale + offset ,//14
        glm::vec3(-0.5f, -0.5f, 0.5f) * scale + offset ,//15
        //Front face
        glm::vec3(-0.5f, -0.5f, 0.5f) * scale + offset ,//16
        glm::vec3(0.5f, -0.5f, 0.5f) * scale + offset ,//17
        glm::vec3(0.5f, 0.5f, 0.5f) * scale + offset ,//18
        glm::vec3(-0.5f, 0.5f, 0.5f) * scale + offset ,//19
        //Back face
        glm::vec3(-0.5f, -0.5f, -0.5f) * scale + offset ,//20
        glm::vec3(0.5f, 0.5f, -0.5f) * scale + offset ,//21
        glm::vec3(0.5f, -0.5f, -0.5f) * scale + offset ,//22
        glm::vec3(-0.5f, 0.5f, -0.5f) * scale + offset ,//23
    };
   cube.texCoords = {
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),

            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
    };

    cube.normals =
    {
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),

            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f),

            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),

            glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f),

            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),

            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
    };


   cube.tangents.resize(24);
    for (int i = 0; i < 36; i += 3)
    {
        std::array triIndices =
        {
            cube.indices[i],
            cube.indices[i + 1],
            cube.indices[i + 2],
        };
        const glm::vec3 edge1 = cube.positions[triIndices[1]] - cube.positions[triIndices[0]];
        const glm::vec3 edge2 = cube.positions[triIndices[2]] - cube.positions[triIndices[0]];
        const glm::vec2 deltaUV1 = cube.texCoords[triIndices[1]] - cube.texCoords[triIndices[0]];
        const glm::vec2 deltaUV2 = cube.texCoords[triIndices[2]] - cube.texCoords[triIndices[0]];

        const float f =
            1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        cube.tangents[triIndices[0]].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        cube.tangents[triIndices[0]].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        cube.tangents[triIndices[0]].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        cube.tangents[triIndices[1]] = cube.tangents[triIndices[0]];
        cube.tangents[triIndices[2]] = cube.tangents[triIndices[0]];

        cube.bitangents[triIndices[0]].x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        cube.bitangents[triIndices[0]].y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        cube.bitangents[triIndices[0]].z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        cube.bitangents[triIndices[1]] = cube.bitangents[triIndices[0]];
        cube.bitangents[triIndices[2]] = cube.bitangents[triIndices[0]];
    }
    return cube;
}
Mesh GenerateSphere(float scale, glm::vec3 offset)
{
    return {};
}
}

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
        mesh.texCoords.emplace_back(texCoords->x, texCoords->y);

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
