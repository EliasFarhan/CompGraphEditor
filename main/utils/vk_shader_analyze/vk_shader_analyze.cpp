
#include "engine/filesystem.h"
#include "proto/renderer.pb.h"
#include "vk/pipeline.h"

#include <spirv_cross/spirv_reflect.hpp>
#include <argh.h>
#include <iostream>
#include <spirv_common.hpp>

#include "utils/log.h"
#include <fmt/format.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

constexpr core::pb::Attribute_Type GetAttributeType(spirv_cross::SPIRType::BaseType type, std::uint32_t vecSize, std::uint32_t columns)
{
    switch(type)
    {
    case spirv_cross::SPIRType::Boolean: return core::pb::Attribute_Type_BOOL;
    case spirv_cross::SPIRType::SByte: break;
    case spirv_cross::SPIRType::UByte: break;
    case spirv_cross::SPIRType::Short: break;
    case spirv_cross::SPIRType::UShort: break;
    case spirv_cross::SPIRType::Int: 
        switch(vecSize)
        {
        case 1: return core::pb::Attribute_Type_INT;
        case 2: return core::pb::Attribute_Type_IVEC2;
        case 3: return core::pb::Attribute_Type_IVEC3;
        case 4: return core::pb::Attribute_Type_IVEC4;
        default: break;
        }
        break;
    case spirv_cross::SPIRType::UInt: break;
    case spirv_cross::SPIRType::Int64: break;
    case spirv_cross::SPIRType::UInt64: break;
    case spirv_cross::SPIRType::AtomicCounter: break;
    case spirv_cross::SPIRType::Half: break;
    case spirv_cross::SPIRType::Float: 
        switch(vecSize)
        {
        case 1: return core::pb::Attribute_Type_FLOAT;
        case 2: return columns == 2 ? core::pb::Attribute_Type_MAT2 : core::pb::Attribute_Type_VEC2;
        case 3: return columns == 3 ? core::pb::Attribute_Type_MAT3 : core::pb::Attribute_Type_VEC3;
        case 4: return columns == 4 ? core::pb::Attribute_Type_MAT4 : core::pb::Attribute_Type_VEC4;
        default: break;
        }
        break;
    case spirv_cross::SPIRType::Double: break;
    case spirv_cross::SPIRType::Struct: break;
    case spirv_cross::SPIRType::Image: break;
    case spirv_cross::SPIRType::SampledImage: break;
    case spirv_cross::SPIRType::Sampler: break;
    case spirv_cross::SPIRType::AccelerationStructure: break;
    case spirv_cross::SPIRType::RayQuery: break;
    case spirv_cross::SPIRType::ControlPointArray: break;
    case spirv_cross::SPIRType::Interpolant: break;
    case spirv_cross::SPIRType::Char: break;
    default: break;
    }
    return core::pb::Attribute_Type_CUSTOM;
}

constexpr std::string GetAttributeTypeName(core::pb::Attribute_Type attributeType)
{
    switch (attributeType)
    {
    case core::pb::Attribute_Type_FLOAT: return "float";
    case core::pb::Attribute_Type_VEC2: return "vec2";
    case core::pb::Attribute_Type_VEC3: return "vec3";
    case core::pb::Attribute_Type_VEC4: return "vec4";
    case core::pb::Attribute_Type_MAT2: return "mat2";
    case core::pb::Attribute_Type_MAT3: return "mat3";
    case core::pb::Attribute_Type_MAT4: return "mat4";
    case core::pb::Attribute_Type_INT: return "int";
    case core::pb::Attribute_Type_IVEC2: return "ivec2";
    case core::pb::Attribute_Type_IVEC3: return "ivec3";
    case core::pb::Attribute_Type_IVEC4: return "ivec4";
    case core::pb::Attribute_Type_BOOL: return "bool";
    case core::pb::Attribute_Type_SAMPLER2D: return "sampler2D";
    case core::pb::Attribute_Type_SAMPLERCUBE: return "samplerCube";
    case core::pb::Attribute_Type_VOID: return "void";
    case core::pb::Attribute_Type_CUSTOM: break;
    default: ;
    }
    return "";
}

int main([[maybe_unused]] int argc, char** argv)
{
    const argh::parser cmd(argv);

    if (cmd.size() < 2)
    {
        LogError("Requires a SPIR-V path as argument");
        return 1;
    }
    const core::DefaultFilesystem filesystem;

    const auto filebuffer = filesystem.LoadFile(cmd[1]);

    try
    {
        const spirv_cross::Compiler compiler(reinterpret_cast<std::uint32_t*>(filebuffer.data), 
            filebuffer.length/sizeof(std::uint32_t));

        auto shaderResources = compiler.get_shader_resources();

        json outputFile;
        
        json inputs = json::array();
        for (auto& input : shaderResources.stage_inputs)
        {
            json inputJson;
            const auto& type = compiler.get_type(input.base_type_id);
            const auto attributeType = GetAttributeType(type.basetype, type.vecsize, type.columns);
            inputJson["type"] = attributeType;
            inputJson["name"] = input.name;
            inputJson["type_name"] = GetAttributeTypeName(attributeType);
            inputs.push_back(inputJson);
        }
        outputFile["inputs"] = inputs;

        json outputs = json::array();
        for (auto& output : shaderResources.stage_outputs)
        {
            json outputJson;
            const auto& type = compiler.get_type(output.base_type_id);
            const auto attributeType = GetAttributeType(type.basetype, type.vecsize, type.columns);
            outputJson["type"] = attributeType;
            outputJson["name"] = output.name;
            outputJson["type_name"] = GetAttributeTypeName(attributeType);
            outputs.push_back(outputJson);
        }
        outputFile["outputs"] = outputs;

        for (auto& sampledImage : shaderResources.sampled_images)
        {
            LogDebug(fmt::format("Sampled Image: {}", sampledImage.name));
        }

        std::unordered_map<std::string, json> structMap;

        std::function<void(spirv_cross::TypeID)> analyzeStruct = [&structMap, &compiler, &analyzeStruct](spirv_cross::TypeID typeId)
        {
            const auto& structType = compiler.get_type(typeId);
            const auto& typeName = compiler.get_fallback_name(typeId);
            if (!structMap.contains(typeName))
            {
                json structJson;
                structJson["name"] = typeName;
                json attributes = json::array();
                int maxAlignement = 0;
                int baseIndex = 0;
                for (std::size_t i = 0; i < structType.member_types.size(); i++)
                {
                    json attributeJson;
                    const auto subTypeId = structType.member_types[i];
                    const auto& subType = compiler.get_type(subTypeId);
                    attributeJson["name"] = compiler.get_member_name(typeId, static_cast<std::uint32_t>(i));
                    const auto attributeType = GetAttributeType(subType.basetype, subType.vecsize, subType.columns);
                    core::TypeInfo typeInfo{};
                    if(attributeType == core::pb::Attribute_Type_CUSTOM)
                    {
                        analyzeStruct(subTypeId);
                        auto& subStructJson = structMap[compiler.get_fallback_name(subTypeId)];
                        typeInfo.size = subStructJson["size"];
                        typeInfo.alignment = subStructJson["alignment"];
                    }
                    else
                    {
                        typeInfo = core::GetTypeInfo(attributeType);
                    }
                    if (baseIndex % typeInfo.alignment != 0)
                    {
                        baseIndex += typeInfo.alignment - baseIndex % typeInfo.alignment;
                    }
                    baseIndex += typeInfo.size;
                    maxAlignement = std::max(typeInfo.alignment, maxAlignement);

                    attributeJson["type"] = attributeType;
                    attributeJson["type_name"] = GetAttributeTypeName(attributeType);
                    attributes.push_back(attributeJson);
                }
                if (baseIndex % maxAlignement != 0)
                {
                    baseIndex += maxAlignement - baseIndex % maxAlignement;
                }
                structJson["attributes"] = attributes;
                structJson["alignment"] = maxAlignement;
                structJson["size"] = baseIndex;
                structMap[typeName] = structJson;
            }
        };
        json uniforms = json::array();
        for (auto& ubo : shaderResources.uniform_buffers)
        {
            json uniformJson;
            const auto& type = compiler.get_type(ubo.base_type_id);
            if (type.basetype == spirv_cross::SPIRType::Struct)
            {
                analyzeStruct( ubo.base_type_id);
            }
            const auto attributeType = GetAttributeType(type.basetype, type.vecsize, type.columns);
            uniformJson["name"] = ubo.name;
            uniformJson["type"] = attributeType;
            uniformJson["type_name"] = attributeType == core::pb::Attribute_Type_CUSTOM ? compiler.get_fallback_name(ubo.base_type_id) :
                GetAttributeTypeName(attributeType);
            uniformJson["push_constant"] = false;
            uniformJson["binding"] = compiler.get_decoration(ubo.id, spv::DecorationBinding);
            uniforms.push_back(uniformJson);
        }
        for (auto& pushConstant : shaderResources.push_constant_buffers)
        {
            json uniformJson;
            const auto& type = compiler.get_type(pushConstant.base_type_id);
            if (type.basetype == spirv_cross::SPIRType::Struct)
            {
                analyzeStruct(pushConstant.base_type_id);
            }
            const auto attributeType = GetAttributeType(type.basetype, type.vecsize, type.columns);
            uniformJson["name"] = pushConstant.name;
            uniformJson["type"] = attributeType;
            uniformJson["type_name"] = attributeType == core::pb::Attribute_Type_CUSTOM ? compiler.get_fallback_name(pushConstant.base_type_id) :
                GetAttributeTypeName(attributeType);
            uniformJson["push_constant"] = true;
            uniformJson["binding"] = -1;
            uniforms.push_back(uniformJson);
        }

        outputFile["uniforms"] = uniforms;

        json structs = json::array();
        for (auto& val : structMap | std::views::values)
        {
            structs.push_back(val);
        }

        outputFile["structs"] = structs;



        std::cout << outputFile.dump(4) << std::endl;
    }
    catch(const spirv_cross::CompilerError& e)
    {
        LogError(fmt::format("Compile Error: {}", e.what()));
        return 1;
    }

    return 0;
}
