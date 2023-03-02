#pragma once

namespace core
{
class Material
{
public:
    virtual std::string_view GetName() const = 0;
};
} // namespace core
