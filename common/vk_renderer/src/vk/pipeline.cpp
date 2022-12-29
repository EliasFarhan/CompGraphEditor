#include "vk/pipeline.h"

#include "vk/engine.h"

namespace gpr5300::vk
{
bool Pipeline::LoadRaterizePipeline(const pb::Pipeline& pipelinePb, Shader& vertexShader, Shader& fragmentShader)
{
    return false;
}

bool Pipeline::LoadComputePipeline(const pb::Pipeline& pipelinePb, Shader& computeShader)
{
    return false;
}

void Pipeline::Bind()
{
    const auto& renderer = GetRenderer();
    vkCmdBindPipeline(renderer.commandBuffers[renderer.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}
}
