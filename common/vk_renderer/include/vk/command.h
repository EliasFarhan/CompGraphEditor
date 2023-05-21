#pragma once

#include <volk.h>

#include "engine.h"
#include "renderer/draw_command.h"
#include "proto/renderer.pb.h"

#include <vector>
#include <array>

namespace vk
{



class UniformManager
{
public:
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

    template<typename T>
    void SetUniform(std::string_view uniformName, const T& uniformValue)
    {
        const auto& uniformData = uniformMap_[uniformName.data()];
        T* ptr = nullptr;
        if (uniformData.uniformType == UniformType::PUSH_CONSTANT)
        {
            ptr = reinterpret_cast<T*>(&pushConstantBuffer_[uniformData.index]);
        }
        else if (uniformData.uniformType == UniformType::UBO)
        {
            ptr = reinterpret_cast<T*>(&uniformBuffer_[uniformData.index]);
        }
        *ptr = uniformValue;
    }

    void Create();
    void Bind();
    void Destroy();

    int materialIndex = -1;
    int pipelineIndex = -1;
    int raytracingPipelineIndex = -1;

private:
    std::unordered_map<std::string, UniformInternalData> uniformMap_;
    std::vector<UniformBufferObject> uniformBuffers_;

    std::vector<std::uint8_t> pushConstantBuffer_;
    std::vector<std::uint8_t> uniformBuffer_;
    std::array<VkDescriptorSet, Engine::MAX_FRAMES_IN_FLIGHT> descriptorSets{};
    VkDescriptorPool descriptorPool;
};

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

    void SetMat3(std::string_view uniformName, const glm::mat3& mat) override;

    void SetAngle(std::string_view uniformName, core::Radian angle) override;

    void Bind() override;

    void PreDrawBind() override;

    void Create();

    void Destroy();
private:
    UniformManager uniformManager_{};
};

class RaytracingCommand
{
public:
    RaytracingCommand(const core::pb::RaytracingCommand& command);
    template<typename T>
    void SetUniform(std::string_view uniformName, const T& uniformValue)
    {
        uniformManager_.SetUniform<T>(uniformName, uniformValue);
    }
    void Create();
    void Destroy();
    void Bind();
    void Dispatch();
private:
    UniformManager uniformManager_{};
    std::vector<Buffer> shaderBindingTables_;
    std::reference_wrapper<const core::pb::RaytracingCommand> commandInfo_;
};
} // namespace vk
