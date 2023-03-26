#pragma once

#include "proto/renderer.pb.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <string_view>
#include <string>


namespace core
{
    
class DrawCommand
{
public:
    DrawCommand(const pb::DrawCommand& drawCommandInfo, int subpassIndex): drawCommandInfo_(drawCommandInfo), subPassIndex_(subpassIndex){}
    virtual ~DrawCommand() = default;
    
    //Uniform functions
    virtual void SetFloat(std::string_view uniformName, float f) = 0;

    virtual void SetInt(std::string_view uniformName, int i)  = 0;

    virtual void SetVec2(std::string_view uniformName, glm::vec2 v) = 0;

    virtual void SetVec3(std::string_view uniformName, glm::vec3 v) = 0;

    virtual void SetVec4(std::string_view uniformName, glm::vec4 v) = 0;

    virtual void SetMat4(std::string_view uniformName, const glm::mat4& mat) = 0;

    [[nodiscard]] std::string_view GetName() const{ return drawCommandInfo_.get().name();}
    int GetMaterialIndex() const { return drawCommandInfo_.get().material_index(); }
    int GetMeshIndex() const { return drawCommandInfo_.get().mesh_index(); }
    const pb::DrawCommand& GetInfo() const { return drawCommandInfo_; }
    int GetSubpassIndex() const { return subPassIndex_; }
    virtual void Bind() = 0;
    /**
     * @brief PreDrawBind is a method that bind transform and apply all deferred changes before drawing
     */
    virtual void PreDrawBind() = 0;
protected:
    std::reference_wrapper<const pb::DrawCommand> drawCommandInfo_;
    int subPassIndex_ = -1;
};

} // namespace core
