#pragma once

#include "generated/renderer_generated.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_float4x4.hpp>

#include <string_view>
#include <string>
#include <array>
#include <span>

namespace core
{

struct Shader
{
    int shaderIndex = -1;
};
constexpr novus::internal::ShaderStage GetShaderStageFromExtension(std::string_view extension)
{
    constexpr std::array<std::string_view, 12> extensions =
    {
        ".vert",
        ".frag",
        ".comp"
    };
    for (int64_t i = 0; i < std::ssize(extensions); i++)
    {
        if(extension == extensions[i])
        {
            return static_cast<novus::internal::ShaderStage>(i);
        }
    }
    return static_cast<novus::internal::ShaderStage>(-1);
}

struct BufferBinding
{
    std::string name;
    int bindingPoint = -1;
};

class Pipeline
{
public:
    virtual void Bind(void* renderData) = 0;

    void SetPipelineName(std::string_view name);
    [[nodiscard]] std::string_view GetPipelineName() const;
    [[nodiscard]] std::span<const BufferBinding> GetBufferBindings() const { return bufferBindings_; }
protected:
    std::vector<BufferBinding> bufferBindings_;
private:
    std::string pipelineName_;
};

struct TypeInfo
{
    int size = 0;
    int alignment = 0;
};

constexpr novus::internal::AttributeType GetAttributeType(std::string_view name)
{
    if (name == "vec4")
    {
        return novus::internal::AttributeType_VEC4;
    }
    if (name == "vec2")
    {
        return novus::internal::AttributeType_VEC2;
    }
    if (name == "vec3")
    {
        return novus::internal::AttributeType_VEC3;
    }
    if (name == "float")
    {
        return novus::internal::AttributeType_FLOAT;
    }
    if (name == "int")
    {
        return novus::internal::AttributeType_INT;
    }
    if (name == "bool")
    {
        return novus::internal::AttributeType_BOOL;
    }
    if (name == "mat4")
    {
        return novus::internal::AttributeType_MAT4;
    }
    if (name == "mat3")
    {
        return novus::internal::AttributeType_MAT3;
    }
    if (name == "mat2")
    {
        return novus::internal::AttributeType_MAT2;
    }
    if (name == "ivec4")
    {
        return novus::internal::AttributeType_IVEC4;
    }
    if (name == "ivec2")
    {
        return novus::internal::AttributeType_IVEC2;
    }
    if (name == "ivec3")
    {
        return novus::internal::AttributeType_IVEC3;
    }
    if (name == "sampler2D")
    {
        return novus::internal::AttributeType_SAMPLER2D;
    }
    if (name == "samplerCube")
    {
        return novus::internal::AttributeType_SAMPLERCUBE;
    }
    if (name == "image2D")
    {
        return novus::internal::AttributeType_IMAGE2D;
    }
    return novus::internal::AttributeType_CUSTOM_STRUCT;
}
/*
constexpr TypeInfo GetTypeInfo(pb::Attribute_Type attributeType, bool array=false)
{
    TypeInfo info{};
    switch(attributeType)
    {
    case pb::Attribute_Type_BOOL:
        info.size = 1;
        info.alignment = 1;
        break;
    case pb::Attribute_Type_INT:
    case pb::Attribute_Type_FLOAT:
        info.size = 4;
        info.alignment = 4;
        break;
    case pb::Attribute_Type_IVEC2:
    case pb::Attribute_Type_VEC2:
        info.size = 8;
        info.alignment = 8;
        break;
    case pb::Attribute_Type_VEC3:
    case pb::Attribute_Type_IVEC3:
    case pb::Attribute_Type_VEC4:
    case pb::Attribute_Type_IVEC4:
        info.size = 16;
        info.alignment = 16;
        break;
    case pb::Attribute_Type_MAT2:
        info.size = 16;
        info.alignment = 8;
        break;
    case pb::Attribute_Type_MAT3:
        info.size = 36;
        info.alignment = 12;
        break;
    case pb::Attribute_Type_MAT4:
        info.size = 64;
        info.alignment = 16;
        break;
    default:
        break;
    }
    return info;
}
*/
} // namespace core
