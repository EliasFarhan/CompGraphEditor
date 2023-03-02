#pragma once

#include "renderer/mesh.h"
#include "vk/mesh.h"

namespace vk
{
class Mesh : public core::Mesh
{
public:
    [[nodiscard]] std::string_view GetName() const override { return ""; }
};
}
