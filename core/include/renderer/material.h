#pragma once

namespace core
{
class Material
{
public:
    virtual ~Material() = default;
    virtual void Bind() = 0;
    virtual std::string_view GetName() const = 0;
};
} // namespace core
