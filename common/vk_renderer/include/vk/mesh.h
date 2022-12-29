#pragma once

#include "renderer/mesh.h"
#include "vk/mesh.h"

namespace gpr5300::vk
{
class Mesh : public gpr5300::Mesh
{
public:
    [[nodiscard]] std::string_view GetName() const override { return ""; }
};
}
