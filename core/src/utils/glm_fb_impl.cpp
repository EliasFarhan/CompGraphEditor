#include "utils/glm_fb_impl.h"
#include "generated/maths_generated.h"

namespace flatbuffers {
novus::maths::Vec3 Pack(const glm::vec3& obj) {
    return {obj.x, obj.y, obj.z};
}

glm::vec3 UnPack(const novus::maths::Vec3& obj) {
    return {obj.x(), obj.y(), obj.z()};
}

novus::maths::Vec2i Pack(const glm::ivec2& obj) {
    return {obj.x, obj.y};
}

glm::ivec2 UnPack(const novus::maths::Vec2i& obj) {
    return {obj.x(), obj.y()};
}
}