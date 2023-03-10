//
// Created by efarhan on 3/10/23.
//

#include <argh.h>
#include <fmt/printf.h>
#include <nlohmann/json.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

int main(int argc, char** argv)
{
    argh::parser cmdl(argv);

    if(cmdl.size() < 2)
    {
        fmt::print(stderr, "Model path required");
        return 1;
    }

    Assimp::Importer importer;
    const auto* scene = importer.ReadFile(cmdl[1], aiProcess_Triangulate);

    //TODO open model, list mesh, count vertices and put it in a json file to be printed in stdout
    return 0;
}