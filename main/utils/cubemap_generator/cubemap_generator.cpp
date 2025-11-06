#include "generated/renderer_generated.h"
#include <argh.h>
#include <print>

#include <fstream>

int main([[maybe_unused]]int argc, char** argv)
{
    argh::parser cmdl;
    cmdl.add_params({ "-o"});
    cmdl.parse(argv);
    if(cmdl.size() < 7)
    {
        std::print(stderr, "Error: generator requires 6 textures path");
        return EXIT_FAILURE;
    }
    novus::renderer::CubemapT cubemap;
    cubemap.texture_paths.reserve(6);
    for(int i = 1; i < 7; i++)
    {
        cubemap.texture_paths.push_back(cmdl[i]);
    }
    std::string cubemapPath = cmdl("o", "cubemap.cube").str();

    flatbuffers::FlatBufferBuilder fbb;
    fbb.Finish(novus::renderer::Cubemap::Pack(fbb, &cubemap));
    uint8_t* buf = fbb.GetBufferPointer();
    size_t size = fbb.GetSize();

    std::ofstream fileOut(cubemapPath.data(), std::ios::binary);
    fileOut.write(reinterpret_cast<char*>(buf), size);
    if (fileOut.fail())
    {
        std::print(stderr, "Could not save cubemap at: {}", cubemapPath);
        return EXIT_FAILURE;
    }
    fileOut.close(); //force write
    return EXIT_SUCCESS;
}