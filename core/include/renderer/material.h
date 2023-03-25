#pragma once

namespace core
{
class Material
{
public:
    virtual ~Material() = default;
    virtual std::string_view GetName() const = 0;
};
} // namespace core
