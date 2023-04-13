#pragma once

#include "renderer/pipeline.h"
#include "engine/scene.h"

#include <vulkan/vulkan.h>

#include <optional>

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
    bool LoadRaterizePipeline(const core::pb::Pipeline& pipelinePb,
                              Shader& vertexShader,
                              Shader& fragmentShader,
                              int pipelineIndex,
        std::optional < std::reference_wrapper<Shader> > geometryShader = std::nullopt,
        std::optional < std::reference_wrapper<Shader> > tesselationControlShader = std::nullopt,
        std::optional < std::reference_wrapper<Shader> > tesselationEvalShader = std::nullopt);
    bool LoadComputePipeline(const core::pb::Pipeline& pipelinePb, 
        Shader& computeShader);
    bool LoadRaytracingPipeline(const core::pb::Pipeline& pipelinePb,
                                const core::pb::RaytracingPipeline raytracingPipelinePb,
                                Shader& rayGenShader,
                                Shader& missHitShader,
                                Shader& closestHitShader,
                                int pipelineIndex,
                                std::optional < std::reference_wrapper<Shader> > anyHitShader = std::nullopt, std::optional < std::reference_wrapper<Shader> > intersectionShadder = std::nullopt);
    void Bind() override;
    void Destroy() const;
    [[nodiscard]] VkPipelineLayout GetLayout() const { return pipelineLayout; }
    [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const{ return descriptorSetLayout; }
    [[nodiscard]] const auto& GetPushConstantDataTable() const {return pushConstantDataTable_;}
private:
    VkPipeline pipeline{};
    VkPipelineLayout pipelineLayout{};
    VkDescriptorSetLayout descriptorSetLayout{};
    std::array<PushConstantData, core::pb::SHADER_TYPE_COUNT> pushConstantDataTable_{};
};
}
