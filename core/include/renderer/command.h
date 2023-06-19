#pragma once

#include "proto/renderer.pb.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <string_view>
#include <string>

#include "maths/angle.h"


namespace core
{

class ModelTransformMatrix
{
public:
    glm::mat4 GetModelTransformMatrix() const;

    constexpr glm::vec3 GetTranslate() const
    {
        return translate_;
    }
    constexpr void SetTranslate(glm::vec3 translate)
    {
        translate_ = translate;
    }
    constexpr glm::vec3 GetScale() const
    {
        return scale_;
    }
    constexpr void SetScale(glm::vec3 scale)
    {
        scale_ = scale;
    }
    constexpr glm::vec3 GetRotation() const
    {
        return rotation_;
    }
    constexpr void SetRotation(glm::vec3 rotation)
    {
        rotation_ = rotation;
    }
private:
    glm::vec3 translate_{ 0.0f };
    glm::vec3 scale_{ 1.0f };
    glm::vec3 rotation_{ 0.0f };
};

class Command
{
public:
    //Uniform functions
    virtual void SetFloat(std::string_view uniformName, float f) = 0;

    virtual void SetInt(std::string_view uniformName, int i) = 0;
    virtual void SetBool(std::string_view uniformName, bool i) = 0;

    virtual void SetVec2(std::string_view uniformName, glm::vec2 v) = 0;

    virtual void SetVec3(std::string_view uniformName, glm::vec3 v) = 0;

    virtual void SetVec4(std::string_view uniformName, glm::vec4 v) = 0;
    virtual void SetMat3(std::string_view uniformName, const glm::mat3& mat) = 0;
    virtual void SetMat4(std::string_view uniformName, const glm::mat4& mat) = 0;

    virtual void SetAngle(std::string_view uniformName, Radian angle) = 0;
    virtual void Bind() = 0;
};

class DrawCommand : public Command
{
public:
    DrawCommand(const pb::DrawCommand& drawCommandInfo, int subpassIndex): drawCommandInfo_(drawCommandInfo), subPassIndex_(subpassIndex)
    {
        if(drawCommandInfo.has_model_transform())
        {
            const auto& modelMatrix = drawCommandInfo.model_transform();
            if(modelMatrix.has_position())
            {
                const auto& translate = modelMatrix.position();
                modelTransformMatrix.SetTranslate({translate.x(), translate.y(), translate.z()});
            }
            if(modelMatrix.has_scale())
            {
                const auto& scale = modelMatrix.scale();
                modelTransformMatrix.SetScale({scale.x(), scale.y(), scale.z()});
            }
            if(modelMatrix.has_euler_angles())
            {
                const auto& rotation = modelMatrix.euler_angles();
                modelTransformMatrix.SetRotation({rotation.x(), rotation.y(), rotation.z()});
            }
        }
    }
    virtual ~DrawCommand() = default;
    
    

    [[nodiscard]] std::string_view GetName() const{ return drawCommandInfo_.get().name();}
    int GetMaterialIndex() const { return drawCommandInfo_.get().material_index(); }
    int GetMeshIndex() const { return drawCommandInfo_.get().mesh_index(); }
    const pb::DrawCommand& GetInfo() const { return drawCommandInfo_; }
    int GetSubpassIndex() const { return subPassIndex_; }
    /**
     * @brief PreDrawBind is a method that bind transform and apply all deferred changes before drawing
     */
    virtual void PreDrawBind() = 0;

    ModelTransformMatrix modelTransformMatrix{};


protected:

    std::reference_wrapper<const pb::DrawCommand> drawCommandInfo_;
    int subPassIndex_ = -1;
};

class ComputeCommand : public Command
{
    
};

} // namespace core
