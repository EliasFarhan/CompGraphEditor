#pragma once

namespace gpr5300
{
class Material
{
public:
    virtual std::string_view GetName() const = 0;
};
} // namespace gpr5300
