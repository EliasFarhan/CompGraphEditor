//
// Created by efarhan on 3/10/23.
//

#include <argh.h>
#include <fmt/printf.h>
#include <nlohmann/json.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using json = nlohmann::json;

int main([[maybe_unused]]int argc, char** argv)
{
    argh::parser cmdl(argv);

    if(cmdl.size() < 2)
    {
        fmt::print(stderr, "Model path required");
        return 1;
    }

    Assimp::Importer importer;
    const auto* scene = importer.ReadFile(cmdl[1], aiProcess_Triangulate);
    json modelJson;
    modelJson["meshes"] = {};
    std::function<void(aiNode*)> func = [&func, scene, &modelJson](aiNode* node){
        for(unsigned i = 0; i < node->mNumMeshes; i++)
        {
            const auto* mesh = scene->mMeshes[node->mMeshes[i]];
            json meshJson;
            meshJson["name"] = mesh->mName.C_Str();
            meshJson["faces"] = mesh->mNumFaces;
            meshJson["vertices"] = mesh->mNumVertices;
            meshJson["bones"] = mesh->mNumBones;
            modelJson["meshes"].push_back(meshJson);

        }

        for(unsigned i = 0; i < node->mNumChildren; i++)
        {
            func(node->mChildren[i]);
        }
    };

    func(scene->mRootNode);

    fmt::print(stdout, "{}", modelJson.dump(4));
    return 0;
}