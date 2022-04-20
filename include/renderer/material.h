#pragma once

#include "renderer/pipeline.h"
#include "texture.h"

namespace gpr5300
{

class Material
{
public:
private:
    Pipeline* pipeline_;
    std::vector<Texture> textures_;
};

}