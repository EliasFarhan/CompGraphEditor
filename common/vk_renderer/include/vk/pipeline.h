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

class Pipeline final : public core::Pipeline
{
public:
    bool LoadRaterizePipeline(const core::pb::Pipeline& pipelinePb, Shader& vertexShader, Shader& fragmentShader);
    bool LoadComputePipeline(const core::pb::Pipeline& pipelinePb, Shader& computeShader);
    void Bind() override;
    void Destroy() const;

private:
    VkPipeline pipeline{};
    VkPipelineLayout pipelineLayout{};
};
}
