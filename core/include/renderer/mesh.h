#pragma once

#include <string_view>

namespace core
{

/**
 * @brief Mesh is an interface to the loading of 3d mesh (aka a bunch a triangle) with
 * positions, texcoords, normal
*/
class Mesh
{
public:
    virtual ~Mesh() = default;
    [[nodiscard]] virtual std::string_view GetName() const = 0;
};
} // namespace core
