#include "vk/draw_command.h"
#include "vk/pipeline.h"
#include "vk/scene.h"
#include "renderer/pipeline.h"
#include "engine/scene.h"
#include "vk/utils.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string_view>
namespace vk
{
void DrawCommand::SetFloat(std::string_view uniformName, float f)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    if(uniformData.pushConstant)
    {
        auto* ptr = reinterpret_cast<float*>(&pushConstantBuffer_[uniformData.index]);
        *ptr = f;
    }
}

void DrawCommand::SetInt(std::string_view uniformName, int i)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    if(uniformData.pushConstant)
    {
        auto* ptr = reinterpret_cast<int*>(&pushConstantBuffer_[uniformData.index]);
        *ptr = i;
    }
}

void DrawCommand::SetVec2(std::string_view uniformName, glm::vec2 v)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    if(uniformData.pushConstant)
    {
        auto* ptr = reinterpret_cast<glm::vec2*>(&pushConstantBuffer_[uniformData.index]);
        *ptr = v;
    }
}

void DrawCommand::SetVec3(std::string_view uniformName, glm::vec3 v)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    if(uniformData.pushConstant)
    {
        auto* ptr = reinterpret_cast<glm::vec3*>(&pushConstantBuffer_[uniformData.index]);
        *ptr = v;
    }
}

void DrawCommand::SetVec4(std::string_view uniformName, glm::vec4 v)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    if(uniformData.pushConstant)
    {
        auto* ptr = reinterpret_cast<glm::vec4*>(&pushConstantBuffer_[uniformData.index]);
        *ptr = v;
    }
}

void DrawCommand::SetMat4(std::string_view uniformName, const glm::mat4 &mat)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    if(uniformData.pushConstant)
    {
        auto* ptr = reinterpret_cast<glm::mat4*>(&pushConstantBuffer_[uniformData.index]);
        *ptr = mat;
    }
}

void DrawCommand::Bind()
{
    //TODO bind pipeline and mesh
}

void DrawCommand::GenerateUniforms()
{
    auto* scene = core::GetCurrentScene();
    const auto& sceneInfo = scene->GetInfo();
    const auto& materialInfo = sceneInfo.materials(drawCommandInfo_.get().material_index());
    const auto& pipelineInfo = sceneInfo.pipelines(materialInfo.pipeline_index());

    std::vector<std::reference_wrapper<const core::pb::Shader>> shaderInfo;
    std::array shaderIndices =
            {
                    pipelineInfo.vertex_shader_index(),
                    pipelineInfo.fragment_shader_index(),
                    pipelineInfo.compute_shader_index(),
                    pipelineInfo.geometry_shader_index()
            };
    for(std::size_t i = 0; i < shaderIndices.size(); i++)
    {
        auto shaderIndex = shaderIndices[i];
        auto& shader = sceneInfo.shaders(shaderIndex);
        if(shader.type() == i)
        {
            shaderInfo.emplace_back(shader);
        }
    }
    int basePushConstantIndex = 0;
    for(auto& shader: shaderInfo)
    {
        for(const auto& uniform: shader.get().uniforms())
        {
            const auto& name = uniform.name();
            UniformInternalData uniformData{};
            if(uniform.push_constant())
            {
                if(uniform.type() != core::pb::Attribute_Type_CUSTOM)
                {
                    const auto typeInfo = core::GetTypeInfo(uniform.type());
                    if(basePushConstantIndex % typeInfo.alignment != 0)
                    {
                        basePushConstantIndex += typeInfo.alignment - basePushConstantIndex % typeInfo.alignment;
                    }
                    uniformData.index = basePushConstantIndex;
                    uniformData.size = typeInfo.size;
                    uniformData.pushConstant = true;
                    basePushConstantIndex += typeInfo.size;
                }
                else
                {
                    for(auto& structType: shader.get().structs())
                    {
                        if(structType.name() == uniform.type_name())
                        {
                            core::TypeInfo typeInfo
                            {
                                .size = structType.size(),
                                .alignment = structType.alignment()
                            };
                            if (basePushConstantIndex % typeInfo.alignment != 0)
                            {
                                basePushConstantIndex += typeInfo.alignment - basePushConstantIndex % typeInfo.alignment;
                            }
                            uniformData.index = basePushConstantIndex;
                            uniformData.size = typeInfo.size;
                            uniformData.pushConstant = true;
                            basePushConstantIndex += typeInfo.size;
                            break;
                        }
                    }
                }
            }
            uniforms_[name] = uniformData;
        }
    }
    pushConstantBuffer_.resize(basePushConstantIndex);
}

void DrawCommand::PreDrawBind()
{
    auto* scene = core::GetCurrentScene();
    const auto& sceneInfo = scene->GetInfo();
    const auto& materialInfo = sceneInfo.materials(drawCommandInfo_.get().material_index());
    const auto& pipeline = static_cast<const Pipeline&>(scene->GetPipeline(materialInfo.pipeline_index()));
    const auto& pushConstantDataTable = pipeline.GetPushConstantDataTable();


    for(std::size_t i = 0; i < pushConstantDataTable.size(); i++)
    {
        if(pushConstantDataTable[i].index != -1)
        {
            const auto shaderType = static_cast<core::pb::ShaderType>(i);
            const auto shaderStage = GetShaderStage(shaderType);
            vkCmdPushConstants(GetCurrentCommandBuffer(),
                               pipeline.GetLayout(),
                               shaderStage,
                               pushConstantDataTable[i].index,
                               pushConstantDataTable[i].size,
                               &pushConstantBuffer_[pushConstantDataTable[i].index]);
        }
    }
}
}