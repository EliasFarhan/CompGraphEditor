//
// Created by unite on 16.07.2025.
//

#ifndef DRAW_COMMAND_H
#define DRAW_COMMAND_H

#include "wasm/neko2.h"

namespace script
{

class DrawCommand
{
public:
    explicit DrawCommand(int64_t drawCommandId): drawCommandId_(drawCommandId) {}

    void Bind() const
    {
        bind_draw_command(drawCommandId_);
    }

    void SetFloat(std::string_view uniformName, float value) const
    {
        set_float(drawCommandId_, uniformName.data(), value);
    }

    void SetVec3(std::string_view uniformName, const glm::vec3& v) const
    {
        set_vec3_local(drawCommandId_, uniformName.data(), &v);
    }
    void SetMat4(std::string_view uniformName, const glm::mat4& value) const
    {
        set_mat4_local(drawCommandId_, uniformName.data(), &value);
    }

    [[nodiscard]] int32_t GetSubpassIndex() const
    {
        return get_subpass_index(drawCommandId_);
    }

    [[nodiscard]] bool EqualsName(std::string_view name) const
    {
        return name_equals(get_name(drawCommandId_), name.data());
    }

    void Draw(int64_t instanceCount = 1) const
    {
        if (instanceCount == 1)
        {
            draw(drawCommandId_);
        }
        else
        {
            draw_instanced(drawCommandId_, instanceCount);
        }
    }
private:
    int64_t drawCommandId_;
};

}

#endif //DRAW_COMMAND_H
