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
    void SetFloat(std::string_view uniformName, float f) override;
    void SetInt(std::string_view uniformName, int i) override;
    void SetVec2(std::string_view uniformName, glm::vec2 v) override;
    void SetVec3(std::string_view uniformName, glm::vec3 v) override;
    void SetVec4(std::string_view uniformName, glm::vec4 v) override;
    void SetMat4(std::string_view uniformName, const glm::mat4& mat) override;

private:
    VkPipeline pipeline{};
    VkPipelineLayout pipelineLayout{};
    std::vector<std::uint8_t> pushConstantData_;
    std::unordered_map<std::string, std::size_t> uniformOffsetMap_;
};
}
