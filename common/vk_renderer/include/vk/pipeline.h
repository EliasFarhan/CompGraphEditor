#pragma once

#include "renderer/pipeline.h"
#include "engine/scene.h"

#include <vulkan/vulkan.h>

namespace vk
{

struct Shader
{
    VkShaderModule module;
    VkShaderStageFlagBits stage;
};

struct PushConstantData
{
    int index = -1;
    int size = 0;
};

class Pipeline final : public core::Pipeline
{
public:
    bool LoadRaterizePipeline(const core::pb::Pipeline& pipelinePb, Shader& vertexShader, Shader& fragmentShader);
    bool LoadComputePipeline(const core::pb::Pipeline& pipelinePb, Shader& computeShader);
    void Bind() override;
    void Destroy() const;
    [[nodiscard]] VkPipelineLayout GetLayout() const { return pipelineLayout; }
    [[nodiscard]] const auto& GetPushConstantDataTable() const {return pushConstantDataTable_;}
private:
    VkPipeline pipeline{};
    VkPipelineLayout pipelineLayout{};
    std::array<PushConstantData, core::pb::SHADER_TYPE_COUNT> pushConstantDataTable_{};
};
}
