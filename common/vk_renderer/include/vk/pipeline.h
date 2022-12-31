#pragma once

#include "renderer/pipeline.h"
#include "engine/scene.h"

#include <vulkan/vulkan.h>

namespace gpr5300::vk
{

struct Shader
{
    VkShaderModule module;
    VkShaderStageFlagBits stage;
};

class Pipeline final : public gpr5300::Pipeline
{
public:
    bool LoadRaterizePipeline(const pb::Pipeline& pipelinePb, Shader& vertexShader, Shader& fragmentShader);
    bool LoadComputePipeline(const pb::Pipeline& pipelinePb, Shader& computeShader);
    void Bind() override;
    void Destroy() const;

private:
    VkPipeline pipeline{};
    VkPipelineLayout pipelineLayout{};
};
}
