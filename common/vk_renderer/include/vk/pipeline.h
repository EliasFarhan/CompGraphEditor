#pragma once

#include "renderer/pipeline.h"
#include "engine/scene.h"

#include <volk.h>

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
    bool LoadRasterizePipeline(const core::pb::Pipeline& pipelinePb,
                              Shader& vertexShader,
                              Shader& fragmentShader,
                              int pipelineIndex,
        std::optional < std::reference_wrapper<Shader> > geometryShader = std::nullopt,
        std::optional < std::reference_wrapper<Shader> > tesselationControlShader = std::nullopt,
        std::optional < std::reference_wrapper<Shader> > tesselationEvalShader = std::nullopt);
    bool LoadComputePipeline(const core::pb::Pipeline& pipelinePb, 
        Shader& computeShader);
    bool LoadRaytracingPipeline(const core::pb::RaytracingPipeline& raytracingPipelinePb,
                                Shader& rayGenShader,
                                Shader& missHitShader,
                                Shader& closestHitShader,
                                std::optional < std::reference_wrapper<Shader> > anyHitShader = std::nullopt, std::optional < std::reference_wrapper<Shader> > intersectionShadder = std::nullopt);
    void Bind() override;
    void Destroy() const;
    [[nodiscard]] VkPipelineLayout GetLayout() const { return pipelineLayout_; }
    [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const{ return descriptorSetLayout_; }
    [[nodiscard]] const auto& GetPushConstantDataTable() const {return pushConstantDataTable_;}
    std::size_t GetGroupCount() const { return shaderGroups_.size(); }
    VkPipeline GetPipeline() const { return pipeline_; }

private:
    VkPipeline pipeline_{};
    VkPipelineLayout pipelineLayout_{};
    VkDescriptorSetLayout descriptorSetLayout_{};
    std::array<PushConstantData, core::pb::SHADER_TYPE_COUNT> pushConstantDataTable_{};

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups_;
    VkPipelineBindPoint pipelineBindPoint_ = VK_PIPELINE_BIND_POINT_MAX_ENUM;
};
}
