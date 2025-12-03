//
// Created by unite on 13.11.2025.
//

#ifndef NEKO2_SHADER_ANALYZER_H
#define NEKO2_SHADER_ANALYZER_H

#include "generated/renderer_generated.h"
#include "generated/internal_renderer_generated.h"

namespace novus
{

struct ShaderAttributeResult
{
    std::vector<internal::BufferStructT> types;
    std::vector<internal::BufferAttributeT> uniformBuffers;
    std::vector<internal::BufferAttributeT> storageBuffers;
    std::vector<internal::ShaderSamplerT> shaderSamplers;
    std::vector<renderer::ShaderInputT> shaderInputs;
};

ShaderAttributeResult GenerateShaderAttributeFromJson(std::string_view jsonPath);
} // namespace novus

#endif // NEKO2_SHADER_ANALYZER_H
