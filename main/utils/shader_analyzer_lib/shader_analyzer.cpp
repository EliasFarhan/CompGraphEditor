//
// Created by unite on 13.11.2025.
//

#include "shader_analyzer.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "renderer/pipeline.h"
using json = nlohmann::json;

namespace novus
{
ShaderAttributeResult GenerateShaderAttributeFromJson(std::string_view jsonPath)
{
    std::ifstream f("example.json");
    json shaderReflectData = json::parse(f);
    ShaderAttributeResult result;
    if (shaderReflectData.contains("types"))
    {
        json types = shaderReflectData["types"];
        std::vector<internal::BufferStructT> structs;
        structs.reserve(types.size());
        for (auto& [type_key, type_value] : types.items())
        {
            internal::BufferStructT bufferStruct;
            bufferStruct.key = type_key;
            bufferStruct.name = type_value["name"];
            for (auto& member : type_value["members"])
            {
                internal::BufferStructAttributeT bufferStructAttribute;
                bufferStructAttribute.name = member["name"];
                bufferStructAttribute.type = member["type"];
                bufferStructAttribute.offset = member["offset"];
                if (member.contains("matrix_stride"))
                {
                    bufferStructAttribute.matrix_stride = member["matrix_stride"];
                }
            }
        }
        result.types = std::move(structs);
    }
    if (shaderReflectData.contains("ubos"))
    {
        std::vector<internal::BufferAttributeT> ubos;
        ubos.reserve(shaderReflectData["ubos"].size());
        for (auto& ubo : shaderReflectData["ubos"])
        {
            internal::BufferAttributeT bufferAttribute;
            bufferAttribute.name = ubo["name"];
            bufferAttribute.type = core::GetAttributeType(ubo["type"]);
            bufferAttribute.block_size = ubo["block_size"];
            bufferAttribute.binding = ubo["binding"];
            bufferAttribute.type_name = ubo["type"];
            bufferAttribute.set = ubo["set"];
            ubos.push_back(bufferAttribute);
        }
        result.uniformBuffers = std::move(ubos);
    }
}
}