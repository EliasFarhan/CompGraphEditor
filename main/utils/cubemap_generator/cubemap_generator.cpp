//
// Created by efarhan on 3/16/23.
//

#include "proto/renderer.pb.h"
#include <argh.h>
#include <fmt/printf.h>

#include <fstream>

int main([[maybe_unused]]int argc, char** argv)
{
    argh::parser cmdl;
    cmdl.add_params({ "-o"});
    cmdl.parse(argv);
    if(cmdl.size() < 7)
    {
        fmt::print(stderr, "Error: generator requires 6 textures path");
        return EXIT_FAILURE;
    }
    core::pb::Cubemap cubemap;
    for(int i = 1; i < 7; i++)
    {
        *cubemap.add_texture_paths() = cmdl[i];
    }
    std::string cubemapPath = cmdl("o", "cubemap.cube").str();

    std::ofstream fileOut(cubemapPath.data(), std::ios::binary);
    if (!cubemap.SerializeToOstream(&fileOut))
    {
        fmt::print(stderr, "Could not save cubemap at: {}", cubemapPath);
        return EXIT_FAILURE;
    }
    fileOut.close(); //force write
    return EXIT_SUCCESS;
}