#pragma once

namespace core
{
class Material
{
public:
    virtual ~Material() = default;
    /**
     * @brief Bind is a method that binds the textures to the correct texture units.
     */
    virtual void Bind() = 0;
    virtual std::string_view GetName() const = 0;
};
} // namespace core
