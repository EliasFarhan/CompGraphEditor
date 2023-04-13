#pragma once

#include <vulkan/vulkan_core.h>

#include "engine.h"
#include "renderer/draw_command.h"
#include "proto/renderer.pb.h"

namespace vk
{


class DrawCommand : public core::DrawCommand
{
public:
    using core::DrawCommand::DrawCommand;
    void SetFloat(std::string_view uniformName, float f) override;

    void SetInt(std::string_view uniformName, int i) override;

    void SetVec2(std::string_view uniformName, glm::vec2 v) override;

    void SetVec3(std::string_view uniformName, glm::vec3 v) override;

    void SetVec4(std::string_view uniformName, glm::vec4 v) override;

    void SetMat4(std::string_view uniformName, const glm::mat4 &mat) override;

    void Bind() override;

    void PreDrawBind() override;

    void GenerateUniforms();

    void Destroy();
private:
    enum class UniformType
    {
        PUSH_CONSTANT,
        UBO,
        SAMPLER,
        NONE
    };
    struct UniformInternalData
    {
        int index = 0;
        int size = 0;
        UniformType uniformType = UniformType::NONE;
        int binding = 0;
    };

    struct UniformBufferObject
    {
        std::array<Buffer, Engine::MAX_FRAMES_IN_FLIGHT> buffers;
        int index = -1;
        int size = 0;
    };

    std::vector<std::uint8_t> pushConstantBuffer_;
    std::vector<std::uint8_t> uniformBuffer_;
    std::unordered_map<std::string, UniformInternalData> uniforms_;
    std::vector<UniformBufferObject> uniformBuffers_;
    std::array<VkDescriptorSet, Engine::MAX_FRAMES_IN_FLIGHT> descriptorSets{};
    VkDescriptorPool descriptorPool;
};
}


