#include "vk/draw_command.h"
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

#include <string_view>
#include <glm/gtx/euler_angles.hpp>


namespace vk
{
void DrawCommand::SetFloat(std::string_view uniformName, float f)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    float* ptr = nullptr;
    if(uniformData.uniformType == UniformType::PUSH_CONSTANT)
    {
        ptr = reinterpret_cast<float*>(&pushConstantBuffer_[uniformData.index]);
    }
    else if(uniformData.uniformType == UniformType::UBO)
    {
        ptr = reinterpret_cast<float*>(&uniformBuffer_[uniformData.index]);
    }
    *ptr = f;

}

void DrawCommand::SetInt(std::string_view uniformName, int i)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    int* ptr = nullptr;
    if (uniformData.uniformType == UniformType::PUSH_CONSTANT)
    {
        ptr = reinterpret_cast<int*>(&pushConstantBuffer_[uniformData.index]);
    }
    else if (uniformData.uniformType == UniformType::UBO)
    {
        ptr = reinterpret_cast<int*>(&uniformBuffer_[uniformData.index]);
    }
    *ptr = i;
}

void DrawCommand::SetVec2(std::string_view uniformName, glm::vec2 v)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    glm::vec2* ptr = nullptr;
    if (uniformData.uniformType == UniformType::PUSH_CONSTANT)
    {
        ptr = reinterpret_cast<glm::vec2*>(&pushConstantBuffer_[uniformData.index]);
    }
    else if (uniformData.uniformType == UniformType::UBO)
    {
        ptr = reinterpret_cast<glm::vec2*>(&uniformBuffer_[uniformData.index]);
    }
    *ptr = v;
}

void DrawCommand::SetVec3(std::string_view uniformName, glm::vec3 v)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    glm::vec3* ptr = nullptr;
    if (uniformData.uniformType == UniformType::PUSH_CONSTANT)
    {
        ptr = reinterpret_cast<glm::vec3*>(&pushConstantBuffer_[uniformData.index]);
    }
    else if (uniformData.uniformType == UniformType::UBO)
    {
        ptr = reinterpret_cast<glm::vec3*>(&uniformBuffer_[uniformData.index]);
    }
    *ptr = v;
}

void DrawCommand::SetVec4(std::string_view uniformName, glm::vec4 v)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    glm::vec4* ptr = nullptr;
    if (uniformData.uniformType == UniformType::PUSH_CONSTANT)
    {
        ptr = reinterpret_cast<glm::vec4*>(&pushConstantBuffer_[uniformData.index]);
    }
    else if (uniformData.uniformType == UniformType::UBO)
    {
        ptr = reinterpret_cast<glm::vec4*>(&uniformBuffer_[uniformData.index]);
    }
    *ptr = v;
}

void DrawCommand::SetMat4(std::string_view uniformName, const glm::mat4& mat)
{
    const auto& uniformData = uniforms_[uniformName.data()];
    
    glm::mat4* ptr = nullptr;
    if (uniformData.uniformType == UniformType::PUSH_CONSTANT)
    {
        ptr = reinterpret_cast<glm::mat4*>(&pushConstantBuffer_[uniformData.index]);
    }
    else if (uniformData.uniformType == UniformType::UBO)
    {
        ptr = reinterpret_cast<glm::mat4*>(&uniformBuffer_[uniformData.index]);
    }
    *ptr = mat;
    
}

void DrawCommand::SetMat3(std::string_view uniformName, const glm::mat3& mat)
{
    const auto& uniformData = uniforms_[uniformName.data()];

    glm::mat3* ptr = nullptr;
    if (uniformData.uniformType == UniformType::PUSH_CONSTANT)
    {
        ptr = reinterpret_cast<glm::mat3*>(&pushConstantBuffer_[uniformData.index]);
    }
    else if (uniformData.uniformType == UniformType::UBO)
    {
        ptr = reinterpret_cast<glm::mat3*>(&uniformBuffer_[uniformData.index]);
    }
    *ptr = mat;
}

void DrawCommand::SetAngle(std::string_view uniformName, core::Radian angle)
{
    const auto& uniformData = uniforms_[uniformName.data()];

    core::Radian* ptr = nullptr;
    if (uniformData.uniformType == UniformType::PUSH_CONSTANT)
    {
        ptr = reinterpret_cast<core::Radian*>(&pushConstantBuffer_[uniformData.index]);
    }
    else if (uniformData.uniformType == UniformType::UBO)
    {
        ptr = reinterpret_cast<core::Radian*>(&uniformBuffer_[uniformData.index]);
    }
    *ptr = angle;
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
        SetMat4("model", modelMatrix);
    }
}

void DrawCommand::GenerateUniforms()
{
    auto* scene = static_cast<Scene*>(core::GetCurrentScene());
    const auto& sceneInfo = scene->GetInfo();
    const auto& materialInfo = sceneInfo.materials(drawCommandInfo_.get().material_index());
    const auto& pipelineInfo = sceneInfo.pipelines(materialInfo.pipeline_index());
    auto& textureManager = static_cast<TextureManager&>(core::GetTextureManager());
    

    std::vector<std::reference_wrapper<const core::pb::Shader>> shaderInfos;
    const std::array shaderIndices =
            {
                    pipelineInfo.vertex_shader_index(),
                    pipelineInfo.fragment_shader_index(),
                    pipelineInfo.compute_shader_index(),
                    pipelineInfo.geometry_shader_index()
            };
    for(std::size_t i = 0; i < shaderIndices.size(); i++)
    {
        const auto shaderIndex = shaderIndices[i];
        if(shaderIndex == -1)
            continue;
        auto& shader = sceneInfo.shaders(shaderIndex);
        if(shader.type() == static_cast<core::pb::ShaderType>(i))
        {
            shaderInfos.emplace_back(shader);
        }
    }

    int basePushConstantIndex = 0;
    int baseUniformIndex = 0;

    std::vector<VkDescriptorPoolSize> poolSizes{};
    std::vector<VkDescriptorImageInfo> imageInfos{};
    std::vector<UniformInternalData> uniformDatas{}; //sampler and ubo
    for(auto& shaderInfo: shaderInfos)
    {
        for(const auto& uniform: shaderInfo.get().uniforms())
        {
            const auto& name = uniform.name();
            UniformInternalData uniformData{};
            if(uniform.push_constant())
            {
                uniformData.uniformType = UniformType::PUSH_CONSTANT;
                if(uniform.type() != core::pb::Attribute_Type_CUSTOM)
                {
                    const auto typeInfo = core::GetTypeInfo(uniform.type());
                    if(basePushConstantIndex % typeInfo.alignment != 0)
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
                                    uniforms_[fmt::format("{}.{}", baseName, structUniform.name())] = internalUniformData;
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
                VkDescriptorPoolSize poolSize;
                if(uniform.type() == core::pb::Attribute_Type_SAMPLER2D || 
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
                    
                    for (auto& materialTexture : materialInfo.textures())
                    {
                        LogDebug(fmt::format("Material Sampler Name: {} Shader Uniform Name: {}", materialTexture.sampler_name(), uniform.name()));
                        if (materialTexture.sampler_name() == uniform.name())
                        {
                            if(!materialTexture.framebuffer_name().empty() && !materialTexture.attachment_name().empty())
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
                else
                {
                    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    poolSize.descriptorCount = static_cast<uint32_t>(Engine::MAX_FRAMES_IN_FLIGHT); 
                    poolSizes.push_back(poolSize);
                    uniformData.uniformType = UniformType::UBO;
                    uniformBuffers_.emplace_back();

                    if(uniform.type() != core::pb::Attribute_Type_CUSTOM)
                    {
                        const auto typeInfo = core::GetTypeInfo(uniform.type());
                        if(baseUniformIndex % typeInfo.alignment != 0)
                        {
                            baseUniformIndex += typeInfo.alignment - baseUniformIndex % typeInfo.alignment;
                        }
                        uniformData.index = baseUniformIndex;
                        uniformData.size = typeInfo.size;
                        baseUniformIndex += typeInfo.size;
                    }
                    else
                    {
                        for(auto& structType: shaderInfo.get().structs())
                        {
                            if(structType.name() != uniform.type_name())
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
                                uniforms_[fmt::format("{}.{}", name, structUniform.name())] = internalUniformData;
                            }
                            baseUniformIndex += typeInfo.size;
                            break;
                        }
                    }
                    uniformDatas.push_back({uniformData.index, uniformData.size, UniformType::UBO, uniform.binding()});

                }
            }
            uniforms_[name] = uniformData;
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

    Pipeline& pipeline = static_cast<Pipeline&>(scene->GetPipeline(materialInfo.pipeline_index()));
    if(pipeline.GetDescriptorSetLayout() == nullptr)
    {
        return;
    }
    std::vector<VkDescriptorSetLayout> layouts(Engine::MAX_FRAMES_IN_FLIGHT, pipeline.GetDescriptorSetLayout());
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
    auto& engine = GetEngine();
    for (size_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++) 
    {
        std::vector<VkWriteDescriptorSet> descriptorWrites{};

        std::vector<VkDescriptorBufferInfo> bufferInfos{};
        bufferInfos.reserve(uniformDatas.size());
        int uboIndex = 0;
        for(auto& uniformData : uniformDatas)
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
            else if(uniformData.uniformType == UniformType::UBO)
            {
                //allocate buffer
                auto& ubo = uniformBuffers_[uboIndex];
                ubo.index = uniformData.index;
                ubo.size = uniformData.size;

                auto& buffer = ubo.buffers[i];
                buffer = engine.CreateBuffer(uniformData.size,
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

void DrawCommand::Destroy()
{
    const auto& driver = GetDriver();
    vkDestroyDescriptorPool(driver.device, descriptorPool, nullptr);
    const auto& allocator = GetAllocator();
    for(auto& buffer: uniformBuffers_)
    {
        for(int i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++)
        {
            vmaDestroyBuffer(allocator, buffer.buffers[i].buffer, buffer.buffers[i].allocation);
        }
    }
    uniformBuffers_.clear();
}

void DrawCommand::PreDrawBind()
{
    auto& engine = GetEngine();
    auto* scene = static_cast<Scene*>(core::GetCurrentScene());
    const auto& sceneInfo = scene->GetInfo();
    const auto& materialInfo = sceneInfo.materials(drawCommandInfo_.get().material_index());
    const auto& pipeline = static_cast<const Pipeline&>(scene->GetPipeline(materialInfo.pipeline_index()));
    const auto& pushConstantDataTable = pipeline.GetPushConstantDataTable();


    for(std::size_t i = 0; i < pushConstantDataTable.size(); i++)
    {
        if(pushConstantDataTable[i].index != -1)
        {
            const auto shaderType = static_cast<core::pb::ShaderType>(i);
            const auto shaderStage = GetShaderStage(shaderType);
            vkCmdPushConstants(GetCurrentCommandBuffer(),
                               pipeline.GetLayout(),
                               shaderStage,
                               pushConstantDataTable[i].index,
                               pushConstantDataTable[i].size,
                               &pushConstantBuffer_[pushConstantDataTable[i].index]);
        }
    }
    //push uniform to buffers
    const auto& allocator = GetAllocator();
    const auto currentFrame = GetRenderer().currentFrame;
    for(auto& ubo : uniformBuffers_)
    {
        void* data;
        vmaMapMemory(allocator, ubo.buffers[currentFrame].allocation, &data);
        std::memcpy(data, &uniformBuffer_[ubo.index], ubo.size);
        vmaUnmapMemory(allocator, ubo.buffers[currentFrame].allocation);
    }
    if (descriptorSets[0] == nullptr)
        return;
    //Bind descriptor sets
    vkCmdBindDescriptorSets(GetCurrentCommandBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipeline.GetLayout(), 
        0, 
        1, 
        &descriptorSets[currentFrame], 
        0, 
        nullptr);
}
}
