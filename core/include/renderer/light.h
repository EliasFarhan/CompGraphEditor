#pragma once
#include "maths/angle.h"
#include <glm/vec3.hpp>

namespace core
{

struct Light
{
    glm::vec3 position;
    glm::vec3 color;
};

struct PointLight : Light
{
    float constant = 0.0f;
    float linear = 0.0f;
    float quadratic = 0.0f;
};

struct SpotLight : Light
{
    glm::vec3 direction;
    Radian outCutoff;
    Radian inCutoff;
};

struct DirectionalLight : Light
{
    glm::vec3 direction;
};

class LightSystem
{
public:


};
} // namespace core
