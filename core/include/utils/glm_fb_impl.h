#ifndef NOVUS_UTILS_GLM_FB_IMPL_H
#define NOVUS_UTILS_GLM_FB_IMPL_H

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

namespace novus::maths
{
struct Vec2i;
struct Vec3;
}

namespace flatbuffers {
novus::maths::Vec3 Pack(const glm::vec3& obj);

glm::vec3 UnPack(const novus::maths::Vec3& obj);


novus::maths::Vec2i Pack(const glm::ivec2& obj);

glm::ivec2 UnPack(const novus::maths::Vec2i& obj);
}

#endif //NOVUS_UTILS_GLM_FB_IMPL_H
