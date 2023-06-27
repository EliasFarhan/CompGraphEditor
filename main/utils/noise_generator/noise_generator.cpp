
#include <argh.h>
#include <glm/vec3.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <random>

int main(int argc, char** argv)
{
    argh::parser cmdl(argv);
    if(cmdl.size()<2)
    {
        return EXIT_FAILURE;
    }
    std::string filename = cmdl[1];
    int width = 4;
    int height = 4;
    if (cmdl({ "--height" }) >> height)
    {
    }
    if(cmdl({ "--width" }) >> width)
    {
    }
    std::vector<glm::vec3> noiseData(width * height);
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dis1(-1.0f, 1.0);
    for (auto& noise : noiseData)
    {
        const auto noiseValue = glm::vec3(dis1(gen), dis1(gen), 0.0f);
        noise = noiseValue;
    }
    const int status = stbi_write_hdr(filename.c_str(), width,height, 3, reinterpret_cast<float*>(noiseData.data()));
    if(status == 0)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
