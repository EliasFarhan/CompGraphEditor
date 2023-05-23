#include "vk/command.h"
#include "vk/pipeline.h"
#include "vk/scene.h"
#include "renderer/pipeline.h"
#include "engine/scene.h"
#include "vk/utils.h"
#include "vk/engine.h"

#include <fmt/format.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <string_view>
namespace vk
{
void UniformManager::Create()
{
    auto* scene = static_cast<Scene*>(core::GetCurrentScene());
    const auto& sceneInfo = scene->GetInfo();
    std::optional<std::reference_wrapper<const core::pb::Material>> materialInfo = 
        materialIndex != -1 ? std::optional{ sceneInfo.materials(materialIndex) } : std::nullopt;
    std::optional<std::reference_wrapper<const core::pb::Pipeline>> pipelineInfo = 
        pipelineIndex != -1 ? std::optional{ sceneInfo.pipelines(pipelineIndex) } : std::nullopt;
    std::optional<std::reference_wrapper<const core::pb::RaytracingPipeline>> raytracingPipelineInfo = 
        raytracingPipelineIndex != -1 ? std::optional{ sceneInfo.raytracing_pipelines(raytracingPipelineIndex) } : std::nullopt;


    std::vector<std::reference_wrapper<const core::pb::Shader>> shaderInfos;
    const std::array shaderIndices =
    {
        pipelineIndex != -1 ? pipelineInfo.value().get().vertex_shader_index() : -1,
        pipelineIndex != -1 ? pipelineInfo.value().get().fragment_shader_index() : -1,
        pipelineIndex != -1 ? pipelineInfo.value().get().compute_shader_index() : -1,
        pipelineIndex != -1 ? pipelineInfo.value().get().geometry_shader_index() : -1,
        pipelineIndex != -1 ? pipelineInfo.value().get().tess_control_shader_index() : -1,
        pipelineIndex != -1 ? pipelineInfo.value().get().tess_eval_shader_index() : -1,
        raytracingPipelineIndex != -1 ? raytracingPipelineInfo.value().get().ray_gen_shader_index() : -1,
        raytracingPipelineIndex != -1 ? raytracingPipelineInfo.value().get().miss_hit_shader_index() : -1,
        raytracingPipelineIndex != -1 ? raytracingPipelineInfo.value().get().any_hit_shader_index() : -1,
        raytracingPipelineIndex != -1 ? raytracingPipelineInfo.value().get().closest_hit_shader_index() : -1,
        raytracingPipelineIndex != -1 ? raytracingPipelineInfo.value().get().intersection_hit_shader_index() : -1
    };
    const std::array shaderTypes =
    {
        core::pb::VERTEX,
        core::pb::FRAGMENT,
        core::pb::COMPUTE,
        core::pb::GEOMETRY,
        core::pb::TESSELATION_CONTROL,
        core::pb::TESSELATION_EVAL,
        core::pb::RAY_GEN,
        core::pb::RAY_MISS,
        core::pb::RAY_ANY_HIT,
        core::pb::RAY_CLOSEST_HIT,
        core::pb::RAY_INTERSECTION,
    };
    for (std::size_t i = 0; i < shaderIndices.size(); i++)
    {
        const auto shaderIndex = shaderIndices[i];
        if (shaderIndex == -1)
            continue;
        auto& shader = sceneInfo.shaders(shaderIndex);
        if (shader.type() == shaderTypes[i])
        {
            shaderInfos.emplace_back(shader);
        }
    }

    int basePushConstantIndex = 0;
    int baseUniformIndex = 0;

    std::vector<VkDescriptorPoolSize> poolSizes{};
    std::vector<VkDescriptorImageInfo> imageInfos{};
    std::vector<UniformInternalData> uniformDatas{}; //sampler and ubo
    for (auto& shaderInfo : shaderInfos)
    {
        for (const auto& uniform : shaderInfo.get().uniforms())
        {
            const auto& name = uniform.name();
            UniformInternalData uniformData{};
            if (uniform.push_constant())
            {
                uniformData.uniformType = UniformType::PUSH_CONSTANT;
                if (uniform.type() != core::pb::Attribute_Type_CUSTOM)
                {
                    const auto typeInfo = core::GetTypeInfo(uniform.type());
                    if (basePushConstantIndex % typeInfo.alignment != 0)
                    {
                        basePushConstantIndex += typeInfo.alignment - basePushConstantIndex % typeInfo.alignment;
                    }
                    uniformData.index = basePushConstantIndex;
                    basePushConstantIndex += typeInfo.size;
                }
                else
                {

                    uniformData.index = basePushConstantIndex;
                    std::function<int(std::string_view, std::string_view, int)> analyzeStruct = [&shaderInfo, &analyzeStruct, this]
                    (std::string_view typeName, std::string_view baseName, int basePushConstantIndex)
                    {
                        for (auto& structType : shaderInfo.get().structs())
                        {
                            if (structType.name() != typeName)
                            {
                                continue;
                            }
                            const core::TypeInfo typeInfo
                            {
                                .size = structType.size(),
                                .alignment = structType.alignment()
                            };
                            if (basePushConstantIndex % typeInfo.alignment != 0)
                            {
                                basePushConstantIndex += typeInfo.alignment - basePushConstantIndex % typeInfo.alignment;
                            }
                            int internalPushConstantIndex = basePushConstantIndex;
                            for (auto& structUniform : structType.attributes())
                            {
                                UniformInternalData internalUniformData{};
                                if (structUniform.type() != core::pb::Attribute_Type_CUSTOM)
                                {
                                    const auto internalTypeInfo = core::GetTypeInfo(structUniform.type());
                                    if (internalPushConstantIndex % internalTypeInfo.alignment != 0)
                                    {
                                        internalPushConstantIndex += internalTypeInfo.alignment - internalPushConstantIndex % internalTypeInfo.alignment;
                                    }
                                    internalUniformData.index = internalPushConstantIndex;
                                    internalUniformData.uniformType = UniformType::PUSH_CONSTANT;
                                    internalPushConstantIndex += internalTypeInfo.size;
                                    uniformMap_[fmt::format("{}.{}", baseName, structUniform.name())] = internalUniformData;
                                }
                                else
                                {
                                    internalPushConstantIndex = analyzeStruct(structUniform.type_name(),
                                        fmt::format("{}.{}", baseName,
                                            structUniform.name()),
                                        internalPushConstantIndex);
                                }
                            }
                            basePushConstantIndex += typeInfo.size;
                            break;
                        }
                        return basePushConstantIndex;
                    };
                    basePushConstantIndex = analyzeStruct(uniform.type_name(), uniform.name(), basePushConstantIndex);
                }
            }
            else
            {
                //Manage texture sampler or uniform buffer object
                VkDescriptorPoolSize poolSize;
                if (uniform.type() == core::pb::Attribute_Type_SAMPLER2D ||
                    uniform.type() == core::pb::Attribute_Type_SAMPLERCUBE)
                {

                    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    poolSize.descriptorCount = static_cast<uint32_t>(Engine::MAX_FRAMES_IN_FLIGHT);

                    poolSizes.push_back(poolSize);


                    uniformData.index = imageInfos.size();
                    uniformData.uniformType = UniformType::SAMPLER;
                    uniformData.binding = uniform.binding();
                    uniformDatas.push_back(uniformData);

                    int textureIndex = -1;
                    std::string_view framebufferName;
                    std::string_view attachmentName;
                    if (materialInfo)
                    {
                        for (auto& materialTexture : materialInfo.value().get().textures())
                        {
                            LogDebug(fmt::format("Material Sampler Name: {} Shader Uniform Name: {}", materialTexture.sampler_name(), uniform.name()));
                            if (materialTexture.sampler_name() == uniform.name())
                            {
                                if (!materialTexture.framebuffer_name().empty() && !materialTexture.attachment_name().empty())
                                {
                                    framebufferName = materialTexture.framebuffer_name();
                                    attachmentName = materialTexture.attachment_name();
                                }
                                else
                                {
                                    textureIndex = materialTexture.texture_index();
                                }
                                break;
                            }
                        }
                    }
                    else
                    {
                        //TODO link samplers in raytracing command
                        continue;
                    }
                    if (textureIndex != -1)
                    {
                        auto& texture = (scene->GetTexture(textureIndex));
                        VkDescriptorImageInfo imageInfo{};
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        imageInfo.imageView = texture.imageView;
                        imageInfo.sampler = texture.sampler;
                        imageInfos.push_back(imageInfo);
                    }
                    else
                    {
                        //use framebuffer render target
                        VkDescriptorImageInfo imageInfo{};
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        auto& framebuffer = scene->GetFramebuffer(framebufferName);
                        auto& renderTarget = framebuffer.GetRenderTarget(attachmentName);
                        imageInfo.imageView = renderTarget.imageView;
                        imageInfo.sampler = renderTarget.sampler;
                        imageInfos.push_back(imageInfo);
                    }
                    continue; //don't put sampler in uniform map
                }
                else if(uniform.type() == core::pb::Attribute_Type_IMAGE2D)
                {

                    uniformData.uniformType = UniformType::IMAGE;
                    uniformData.binding = uniform.binding();
                    uniformData.index = imageInfos.size();
                    uniformDatas.push_back(uniformData);

                    VkDescriptorImageInfo storageImageDescriptor{};
                    storageImageDescriptor.imageView = scene->GetStorageImage().imageView;
                    storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

                    imageInfos.push_back(storageImageDescriptor);

                    continue;
                }
                else if(uniform.type() == core::pb::Attribute_Type_ACCELERATION_STRUCT)
                {
                    poolSize.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                    poolSize.descriptorCount = static_cast<uint32_t>(Engine::MAX_FRAMES_IN_FLIGHT);
                    poolSizes.push_back(poolSize);

                    uniformData.uniformType = UniformType::ACCELERATION_STRUCT;
                    uniformData.binding = uniform.binding();

                    uniformDatas.push_back(uniformData);
                    continue;
                }
                else
                {
                    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    poolSize.descriptorCount = static_cast<uint32_t>(Engine::MAX_FRAMES_IN_FLIGHT);
                    poolSizes.push_back(poolSize);
                    uniformData.uniformType = UniformType::UBO;
                    uniformBuffers_.emplace_back();

                    if (uniform.type() != core::pb::Attribute_Type_CUSTOM)
                    {
                        const auto typeInfo = core::GetTypeInfo(uniform.type());
                        if (baseUniformIndex % typeInfo.alignment != 0)
                        {
                            baseUniformIndex += typeInfo.alignment - baseUniformIndex % typeInfo.alignment;
                        }
                        uniformData.index = baseUniformIndex;
                        uniformData.size = typeInfo.size;
                        baseUniformIndex += typeInfo.size;
                    }
                    else
                    {
                        for (auto& structType : shaderInfo.get().structs())
                        {
                            if (structType.name() != uniform.type_name())
                            {
                                continue;
                            }
                            const core::TypeInfo typeInfo
                            {
                                .size = structType.size(),
                                .alignment = structType.alignment()
                            };
                            if (baseUniformIndex % typeInfo.alignment != 0)
                            {
                                baseUniformIndex += typeInfo.alignment - baseUniformIndex % typeInfo.alignment;
                            }
                            uniformData.index = baseUniformIndex;
                            uniformData.size = typeInfo.size;

                            int internalUniformIndex = baseUniformIndex;
                            for (auto& structUniform : structType.attributes())
                            {
                                UniformInternalData internalUniformData{};
                                const auto internalTypeInfo = core::GetTypeInfo(structUniform.type());
                                if (internalUniformIndex % internalTypeInfo.alignment != 0)
                                {
                                    internalUniformIndex += internalTypeInfo.alignment - internalUniformIndex % internalTypeInfo.alignment;
                                }
                                internalUniformData.index = internalUniformIndex;
                                internalUniformData.size = internalTypeInfo.size;
                                internalUniformData.uniformType = UniformType::UBO;
                                internalUniformIndex += internalTypeInfo.size;
                                uniformMap_[fmt::format("{}.{}", name, structUniform.name())] = internalUniformData;
                            }
                            baseUniformIndex += typeInfo.size;
                            break;
                        }
                    }
                    uniformDatas.push_back({ uniformData.index, uniformData.size, UniformType::UBO, uniform.binding() });

                }
            }
            uniformMap_[name] = uniformData;
        }
    }
    uniformBuffer_.resize(baseUniformIndex);
    pushConstantBuffer_.resize(basePushConstantIndex);

    const auto& driver = GetDriver();
    //Generate descriptor pool

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.empty() ? nullptr : poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(Engine::MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(driver.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        LogError("Failed to create descriptor pool!");
        return;
    }

    //Allocate descriptor sets
    const auto descriptorSetLayout = GetDescriptorSetLayout(pipelineIndex, raytracingPipelineIndex);
    if (descriptorSetLayout == VK_NULL_HANDLE)
    {
        return;
    }
    std::vector<VkDescriptorSetLayout> layouts(Engine::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(Engine::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(driver.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        LogError("Failed to allocate descriptor sets!");
        return;
    }
    for (size_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++)
    {
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
        std::vector<VkDescriptorBufferInfo> bufferInfos{};
        std::vector< VkDescriptorImageInfo> imageDescriptors{};
        imageDescriptors.reserve(uniformDatas.size());
        bufferInfos.reserve(uniformDatas.size());
        int uboIndex = 0;
        for (auto& uniformData : uniformDatas)
        {
            if (uniformData.uniformType == UniformType::SAMPLER)
            {
                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = descriptorSets[i];
                descriptorWrite.dstBinding = uniformData.binding;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo = &imageInfos[uniformData.index];
                descriptorWrites.push_back(descriptorWrite);
            }
            else if(uniformData.uniformType == UniformType::IMAGE)
            {
                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = descriptorSets[i];
                descriptorWrite.dstBinding = uniformData.binding;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo = &imageInfos[uniformData.index];
                descriptorWrites.push_back(descriptorWrite);
            }
            else if(uniformData.uniformType == UniformType::ACCELERATION_STRUCT)
            {
                const auto& tlas = scene->GetAccelerationStructure(accelerationStructureIndex);
                descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
                descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
                descriptorAccelerationStructureInfo.pAccelerationStructures = &tlas.GetHandle();

                VkWriteDescriptorSet accelerationStructureWrite{};
                accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                // The specialized acceleration structure descriptor has to be chained
                accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
                accelerationStructureWrite.dstSet = descriptorSets[i];
                accelerationStructureWrite.dstBinding = uniformData.binding;
                accelerationStructureWrite.descriptorCount = 1;
                accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                descriptorWrites.push_back(accelerationStructureWrite);
            }
            else if (uniformData.uniformType == UniformType::UBO)
            {
                //allocate buffer
                auto& ubo = uniformBuffers_[uboIndex];
                ubo.index = uniformData.index;
                ubo.size = uniformData.size;

                auto& buffer = ubo.buffers[i];
                buffer = CreateBuffer(uniformData.size,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                //describe buffer
                bufferInfos.emplace_back();
                auto& bufferInfo = bufferInfos.back();
                bufferInfo.buffer = buffer.buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = uniformData.size;
                //descriptor write

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = descriptorSets[i];
                descriptorWrite.dstBinding = uniformData.binding;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;
                descriptorWrites.push_back(descriptorWrite);
                uboIndex++;
            }
        }

        vkUpdateDescriptorSets(driver.device,
            static_cast<uint32_t>(
                descriptorWrites.size()),
            descriptorWrites.data(),
            0,
            nullptr);
    }
}

void UniformManager::Bind()
{
    auto* scene = static_cast<Scene*>(core::GetCurrentScene());
    const auto& sceneInfo = scene->GetInfo();
    const auto& pipeline = pipelineIndex != -1 ? static_cast<const Pipeline&>(scene->GetPipeline(pipelineIndex)) : scene->GetRaytracingPipeline(raytracingPipelineIndex);
    const auto& pipelineLayout = pipeline.GetLayout();
    const auto& pushConstantDataTable = pipeline.GetPushConstantDataTable();


    for (std::size_t i = 0; i < pushConstantDataTable.size(); i++)
    {
        if (pushConstantDataTable[i].index != -1)
        {
            const auto shaderType = static_cast<core::pb::ShaderType>(i);
            const auto shaderStage = GetShaderStage(shaderType);
            vkCmdPushConstants(GetCurrentCommandBuffer(),
                pipelineLayout,
                shaderStage,
                pushConstantDataTable[i].index,
                pushConstantDataTable[i].size,
                &pushConstantBuffer_[pushConstantDataTable[i].index]);
        }
    }
    //push uniform to buffers
    const auto currentFrame = GetRenderer().currentFrame;
    for (auto& ubo : uniformBuffers_)
    {
        void* data = ubo.buffers[currentFrame].Map();
        std::memcpy(data, &uniformBuffer_[ubo.index], ubo.size);
        ubo.buffers[currentFrame].Unmap();
    }
    if (descriptorSets[0] == nullptr)
        return;
    //Bind descriptor sets
    vkCmdBindDescriptorSets(GetCurrentCommandBuffer(),
        pipeline.GetBindPoint(),
        pipelineLayout,
        0,
        1,
        &descriptorSets[currentFrame],
        0,
        nullptr);
}

void UniformManager::Destroy()
{
    const auto& driver = GetDriver();
    vkDestroyDescriptorPool(driver.device, descriptorPool, nullptr);
    for (auto& buffer : uniformBuffers_)
    {
        for (int i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++)
        {
            DestroyBuffer(buffer.buffers[i]);
        }
    }
    uniformBuffers_.clear();
}

void DrawCommand::Destroy()
{
    uniformManager_.Destroy();
}

RaytracingCommand::RaytracingCommand(const core::pb::RaytracingCommand& command) : commandInfo_(command)
{
}

void RaytracingCommand::Create()
{
    auto* scene = static_cast<vk::Scene*>(core::GetCurrentScene());
    const auto pipelineIndex = commandInfo_.get().pipeline_index();
    auto& pipeline = scene->GetRaytracingPipeline(pipelineIndex);
    uniformManager_.raytracingPipelineIndex = pipelineIndex;
    uniformManager_.accelerationStructureIndex = commandInfo_.get().top_level_acceleration_structure_index();
    uniformManager_.Create();

    const auto& rayTracingPipelineProperties = GetRayTracingPipelineProperties();
    const auto shaderGroupCount = pipeline.GetGroupCount();
    const std::uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
    const std::uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
    const std::uint32_t groupCount = static_cast<std::uint32_t>(shaderGroupCount);
    const std::uint32_t sbtSize = groupCount * handleSizeAligned;
    shaderBindingTables_.resize(groupCount);

    std::vector<uint8_t> shaderHandleStorage(sbtSize);
    if(vkGetRayTracingShaderGroupHandlesKHR(GetDriver().device, pipeline.GetPipeline(), 0, groupCount, sbtSize, shaderHandleStorage.data()) != VK_SUCCESS)
    {
        LogError("Could not create RayTracing Shader Group Handles");
        return;
    }

    constexpr VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    constexpr VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    for(std::uint32_t i = 0; i < groupCount; i++)
    {
        auto& shaderBindingTable = shaderBindingTables_[i];
        shaderBindingTable = CreateBuffer(handleSize, bufferUsageFlags, memoryUsageFlags);
        void* data = shaderBindingTable.Map();
        std::memcpy(data, shaderHandleStorage.data() + handleSizeAligned*i, handleSize);
        shaderBindingTable.Unmap();
    }
    
    VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
    raygenShaderSbtEntry.deviceAddress = GetBufferDeviceAddress(shaderBindingTables_[0].buffer);
    raygenShaderSbtEntry.stride = handleSizeAligned;
    raygenShaderSbtEntry.size = handleSizeAligned;
    this->shaderBindingStridedAddresses_[0] = raygenShaderSbtEntry;

    VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
    missShaderSbtEntry.deviceAddress = GetBufferDeviceAddress(shaderBindingTables_[1].buffer);
    missShaderSbtEntry.stride = handleSizeAligned;
    missShaderSbtEntry.size = handleSizeAligned;
    shaderBindingStridedAddresses_[1] = missShaderSbtEntry;

    VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
    hitShaderSbtEntry.deviceAddress = GetBufferDeviceAddress(shaderBindingTables_[2].buffer);
    hitShaderSbtEntry.stride = handleSizeAligned;
    hitShaderSbtEntry.size = handleSizeAligned;
    shaderBindingStridedAddresses_[2] = hitShaderSbtEntry;
}

void RaytracingCommand::Destroy()
{
    uniformManager_.Destroy();
    for(auto& shaderBindingTable : shaderBindingTables_)
    {
        shaderBindingTable.Destroy();
    }
    shaderBindingTables_.clear();
}

void RaytracingCommand::Bind()
{
    auto* scene = static_cast<Scene*>(core::GetCurrentScene());
    const auto& sceneInfo = scene->GetInfo();
    auto& pipeline = scene->GetRaytracingPipeline(commandInfo_.get().pipeline_index());

    pipeline.Bind();
    uniformManager_.Bind();
}

void RaytracingCommand::Dispatch()
{
    auto* scene = static_cast<Scene*>(core::GetCurrentScene());
    const auto& sceneInfo = scene->GetInfo();
    auto& renderer = GetRenderer();
    auto& pipeline = scene->GetRaytracingPipeline(commandInfo_.get().pipeline_index());
    const auto windowSize = core::GetWindowSize();
    vkCmdTraceRaysKHR(
        renderer.commandBuffers[renderer.imageIndex],
        &shaderBindingStridedAddresses_[0],
        &shaderBindingStridedAddresses_[1],
        &shaderBindingStridedAddresses_[2],
        &shaderBindingStridedAddresses_[3],
        windowSize.x,
        windowSize.y,
        sceneInfo.raytracing_pipelines(commandInfo_.get().pipeline_index()).max_recursion_depth());
}

void DrawCommand::SetFloat(std::string_view uniformName, float f)
{
    uniformManager_.SetUniform(uniformName, f);
}

void DrawCommand::SetInt(std::string_view uniformName, int i)
{
    uniformManager_.SetUniform(uniformName, i);
}

void DrawCommand::SetVec2(std::string_view uniformName, glm::vec2 v)
{
    uniformManager_.SetUniform(uniformName, v);
}

void DrawCommand::SetVec3(std::string_view uniformName, glm::vec3 v)
{
    uniformManager_.SetUniform(uniformName, v);
}

void DrawCommand::SetVec4(std::string_view uniformName, glm::vec4 v)
{
    uniformManager_.SetUniform(uniformName, v);
}

void DrawCommand::SetMat4(std::string_view uniformName, const glm::mat4& mat)
{

    uniformManager_.SetUniform<glm::mat4>(uniformName, mat);
}

void DrawCommand::SetMat3(std::string_view uniformName, const glm::mat3& mat)
{
    uniformManager_.SetUniform<glm::mat3>(uniformName, mat);
}

void DrawCommand::SetAngle(std::string_view uniformName, core::Radian angle)
{
    uniformManager_.SetUniform(uniformName, angle);
}

void DrawCommand::Bind()
{
    auto* scene = static_cast<Scene*>(core::GetCurrentScene());
    const auto& sceneInfo = scene->GetInfo();
    const auto& materialInfo = sceneInfo.materials(drawCommandInfo_.get().material_index());
    auto& pipeline = scene->GetPipeline(materialInfo.pipeline_index());

    pipeline.Bind();

    VkDeviceSize offsets[] = { 0 };
    const auto meshIndex = GetMeshIndex();
    if (meshIndex != -1 && sceneInfo.meshes(meshIndex).primitve_type() != core::pb::Mesh_PrimitveType_NONE)
    {
        auto& renderer = GetRenderer();
        const auto& vertexBuffer = scene->GetVertexBuffers()[meshIndex];
        vkCmdBindVertexBuffers(renderer.commandBuffers[renderer.imageIndex], 0, 1, &vertexBuffer.vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(renderer.commandBuffers[renderer.imageIndex], vertexBuffer.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    }
    //TODO bind textures samplers

    if (drawCommandInfo_.get().has_model_transform())
    {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        const auto& modelTransform = drawCommandInfo_.get().model_transform();
        if (modelTransform.has_euler_angles())
        {
            const auto& eulerAngles = modelTransform.euler_angles();
            modelMatrix *= glm::orientate4(glm::vec3(eulerAngles.x(), eulerAngles.y(), eulerAngles.z()));
        }
        if (modelTransform.has_scale())
        {
            const auto& scale = modelTransform.scale();
            modelMatrix = glm::scale(modelMatrix, glm::vec3(scale.x(), scale.y(), scale.z()));
        }
        if (modelTransform.has_position())
        {
            const auto& translate = modelTransform.position();
            modelMatrix = glm::translate(modelMatrix, glm::vec3(translate.x(), translate.y(), translate.z()));
        }
        //SetMat4("model", modelMatrix);
    }
}

void DrawCommand::Create()
{
    uniformManager_.materialIndex = drawCommandInfo_.get().material_index();
    const auto& sceneInfo = core::GetCurrentScene()->GetInfo();
    uniformManager_.pipelineIndex = sceneInfo.materials(uniformManager_.materialIndex).pipeline_index();
    uniformManager_.Create();
}

void DrawCommand::PreDrawBind()
{
    uniformManager_.Bind();
}   
} // namespace vk
