#pragma once

#include <string_view>

namespace gpr5300
{
class Mesh
{
public:
    [[nodiscard]] virtual std::string_view GetName() const = 0;
};
} // namespace gpr5300
