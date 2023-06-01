#pragma once

#include "proto/renderer.pb.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_float4x4.hpp>

#include <string_view>
#include <string>
#include <array>

namespace core
{

class Shader
{

};

constexpr pb::ShaderType GetTypeFromExtension(std::string_view extension)
{
    constexpr std::array<std::string_view, 12> extensions =
    {
        ".vert",
        ".frag",
        ".comp",
        ".geom",
        ".tesc",
        ".tese",
        ".rgen",
        ".rint",
        ".rahit",
        ".rchit",
        ".rmiss",
        ".rcall"
    };
    for(std::size_t i = 0; i < extensions.size(); i++)
    {
        if(extension == extensions[i])
        {
            return static_cast<pb::ShaderType>(i);
        }
    }
    return pb::SHADER_TYPE_COUNT;
}

class Pipeline
{
public:
    virtual ~Pipeline() = default;
    virtual void Bind() = 0;

    void SetPipelineName(std::string_view name);
    [[nodiscard]] std::string_view GetPipelineName() const;
private:
    std::string pipelineName_;

};

struct TypeInfo
{
    int size = 0;
    int alignment = 0;
};

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
} // namespace core
