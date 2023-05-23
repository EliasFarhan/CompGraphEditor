#include "vk/pipeline.h"

#include "vk/engine.h"
#include "vk/scene.h"
#include "vk/utils.h"

namespace vk
{

bool Pipeline::LoadRasterizePipeline(const core::pb::Pipeline& pipelinePb,
                                    Shader& vertexShader,
                                    Shader& fragmentShader,
                                    int pipelineIndex,
                                    std::optional < std::reference_wrapper<Shader> > geometryShader,
                                    std::optional < std::reference_wrapper<Shader> > tesselationControlShader,
                                    std::optional < std::reference_wrapper<Shader> > tesselationEvalShader)
{
    auto* scene = core::GetCurrentScene();
    const auto& sceneInfo = scene->GetInfo();
    auto& swapchain = GetSwapchain();
    auto& driver = GetDriver();
    pipelineBindPoint_ = VK_PIPELINE_BIND_POINT_GRAPHICS;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
        {
            {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage = vertexShader.stage, .module = vertexShader.module,.pName = "main"},
            {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage = fragmentShader.stage, .module = fragmentShader.module,.pName = "main"}
        } };
    if(geometryShader)
    {
        shaderStages.push_back({ 
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = geometryShader.value().get().stage,
            .module = geometryShader.value().get().module,
            .pName = "main" });
    }
    if(tesselationControlShader)
    {
        shaderStages.push_back({ 
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = tesselationControlShader.value().get().stage,
            .module = tesselationControlShader.value().get().module,
            .pName = "main" });

    }
    if(tesselationEvalShader)
    {
        shaderStages.push_back({ 
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = tesselationEvalShader.value().get().stage,
            .module = tesselationEvalShader.value().get().module,
            .pName = "main" });
    }
    
    VkVertexInputBindingDescription vertexInputBindingDescription{};
    vertexInputBindingDescription.binding = 0;
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexInputBindingDescription.stride = sizeof(core::Vertex);


    std::array<VkVertexInputAttributeDescription, 5> vertexAttributeDescriptors;
    auto& positionAttribute = vertexAttributeDescriptors[0];
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.offset = offsetof(core::Vertex, position);

    auto& texCoordsAttribute = vertexAttributeDescriptors[1];
    texCoordsAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    texCoordsAttribute.binding = 0;
    texCoordsAttribute.location = 1;
    texCoordsAttribute.offset = offsetof(core::Vertex, texCoords);

    auto& normalAttribute = vertexAttributeDescriptors[2];
    normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normalAttribute.binding = 0;
    normalAttribute.location = 2;
    normalAttribute.offset = offsetof(core::Vertex, normal);

    auto& tangentAttribute = vertexAttributeDescriptors[3];
    tangentAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    tangentAttribute.binding = 0;
    tangentAttribute.location = 3;
    tangentAttribute.offset = offsetof(core::Vertex, tangent);

    auto& bitangentAttribute = vertexAttributeDescriptors[4];
    bitangentAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    bitangentAttribute.binding = 0;
    bitangentAttribute.location = 4;
    bitangentAttribute.offset = offsetof(core::Vertex, bitangent);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = pipelinePb.in_vertex_attributes_size() == 0 ? 0:1;
    vertexInputInfo.pVertexBindingDescriptions = pipelinePb.in_vertex_attributes_size() == 0 ? VK_NULL_HANDLE : &vertexInputBindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = pipelinePb.in_vertex_attributes_size();
    vertexInputInfo.pVertexAttributeDescriptions =  vertexAttributeDescriptors.data(); // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain.extent.width);
    viewport.height = static_cast<float>(swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain.extent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    if (pipelinePb.enable_culling())
    {
        switch (pipelinePb.cull_face())
        {
        case core::pb::Pipeline_CullFace_BACK:
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case core::pb::Pipeline_CullFace_FRONT:
            rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
            break;
        case core::pb::Pipeline_CullFace_FRONT_AND_BACK:
            rasterizer.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
            break;
        default: break;
        }
    }
    else
    {
        rasterizer.cullMode = VK_CULL_MODE_NONE;
    }
    switch (pipelinePb.front_face())
    {
    case core::pb::Pipeline_FrontFace_CLOCKWISE:
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        break;
    case core::pb::Pipeline_FrontFace_COUNTER_CLOCKWISE:
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        break;
    default:;
    }
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    if(pipelinePb.blend_enable())
    {
        switch(pipelinePb.blending_source_factor())
        {
        case core::pb::Pipeline_BlendFunc_BLEND_ZERO: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            break;
        case core::pb::Pipeline_BlendFunc_ONE:
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            break;
        case core::pb::Pipeline_BlendFunc_SRC_COLOR: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC_COLOR: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_DST_COLOR: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_DST_COLOR: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_SRC_ALPHA: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC_ALPHA: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_DST_ALPHA: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_DST_ALPHA: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_CONSTANT_COLOR: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_CONSTANT_COLOR: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_CONSTANT_ALPHA: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_CONSTANT_ALPHA: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_SRC_ALPHA_SATURATE: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
            break;
        case core::pb::Pipeline_BlendFunc_SRC1_COLOR: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC1_COLOR: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_SRC1_ALPHA: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC1_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC1_ALPHA: 
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
            break;
        default: break;
        }
        switch(pipelinePb.blending_destination_factor())
        {
        case core::pb::Pipeline_BlendFunc_BLEND_ZERO: 
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            break;
        case core::pb::Pipeline_BlendFunc_ONE:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            break;
        case core::pb::Pipeline_BlendFunc_SRC_COLOR:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC_COLOR:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_DST_COLOR:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_DST_COLOR:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_SRC_ALPHA:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC_ALPHA:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_DST_ALPHA:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_DST_ALPHA:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_CONSTANT_COLOR:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_CONSTANT_COLOR:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_CONSTANT_ALPHA: 
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_CONSTANT_ALPHA:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_SRC_ALPHA_SATURATE: 
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
            break;
        case core::pb::Pipeline_BlendFunc_SRC1_COLOR: 
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC1_COLOR: 
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
            break;
        case core::pb::Pipeline_BlendFunc_SRC1_ALPHA:
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC1_ALPHA;
            break;
        case core::pb::Pipeline_BlendFunc_ONE_MINUS_SRC1_ALPHA: 
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
            break;
        default: break;
        }
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    }
    else
    {
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    }


    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    //generate uniform push constant table
    std::vector<std::reference_wrapper<const core::pb::Shader>> shaderInfo;
    std::array shaderIndices =
    {
        pipelinePb.vertex_shader_index(),
        pipelinePb.fragment_shader_index(),
        pipelinePb.geometry_shader_index(),
        pipelinePb.tess_control_shader_index(),
        pipelinePb.tess_eval_shader_index(),
    };
    std::array shaderTypes =
    {
        core::pb::VERTEX,
        core::pb::FRAGMENT,
        core::pb::GEOMETRY,
        core::pb::TESSELATION_CONTROL,
        core::pb::TESSELATION_EVAL
    };
    for(std::size_t i = 0; i < shaderIndices.size(); i++)
    {
        auto shaderIndex = shaderIndices[i];
        if(shaderIndex == -1)
            continue;
        auto& shader = sceneInfo.shaders(shaderIndex);
        if(shader.type() == shaderTypes[i])
        {
            shaderInfo.emplace_back(shader);
        }
    }
    int basePushConstantIndex = 0;
    std::vector<VkPushConstantRange> pushConstantRanges{};
    std::vector<VkDescriptorSetLayoutBinding > descriptorSetLayoutBindings;

    for(auto& shader: shaderInfo)
    {
        for(const auto& uniform: shader.get().uniforms())
        {
            auto& uniformData = pushConstantDataTable_[uniform.stage()];
            if(uniform.push_constant())
            {
                VkPushConstantRange pushConstantRange{};
                if(uniform.type() != core::pb::Attribute_Type_CUSTOM)
                {
                    const auto typeInfo = core::GetTypeInfo(uniform.type());
                    if(basePushConstantIndex % typeInfo.alignment != 0)
                    {
                        basePushConstantIndex += typeInfo.alignment - basePushConstantIndex % typeInfo.alignment;
                    }
                    uniformData.index = basePushConstantIndex;
                    uniformData.size = typeInfo.size;
                    pushConstantRange.offset = basePushConstantIndex;
                    pushConstantRange.size = typeInfo.size;
                    basePushConstantIndex += typeInfo.size;

                }
                else
                {
                    for(auto& structType : shader.get().structs())
                    {
                        if(structType.name() == uniform.type_name())
                        {
                            core::TypeInfo typeInfo
                            {
                                .size = structType.size(),
                                .alignment = structType.alignment()
                            };
                            if(basePushConstantIndex % typeInfo.alignment != 0)
                            {
                                basePushConstantIndex += typeInfo.alignment - basePushConstantIndex % typeInfo.alignment;
                            }
                            uniformData.index = basePushConstantIndex;
                            uniformData.size = typeInfo.size;
                            pushConstantRange.offset = basePushConstantIndex;
                            pushConstantRange.size = typeInfo.size;
                            basePushConstantIndex += typeInfo.size;
                            break;
                        }
                    }
                }
                pushConstantRange.stageFlags = GetShaderStage(uniform.stage());
                pushConstantRanges.push_back(pushConstantRange);
            }
            else
            {
                VkDescriptorSetLayoutBinding layoutBinding{};
                if(uniform.type() == core::pb::Attribute_Type_SAMPLER2D || 
                    uniform.type() == core::pb::Attribute_Type_SAMPLERCUBE)
                {
                    //sampler
                    layoutBinding.binding = uniform.binding();
                    layoutBinding.descriptorCount = 1;
                    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    layoutBinding.pImmutableSamplers = nullptr;
                    layoutBinding.stageFlags = GetShaderStage(uniform.stage());
                }
                else
                {
                    //ubo
                    layoutBinding.binding = uniform.binding();
                    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    layoutBinding.descriptorCount = 1;
                    layoutBinding.stageFlags = GetShaderStage(uniform.stage());
                    layoutBinding.pImmutableSamplers = nullptr;
                }
                descriptorSetLayoutBindings.push_back(layoutBinding);
            }
        }
    }
    if(basePushConstantIndex > 128)
    {
        LogWarning("Push Constant Size is bigger than 128 bytes");
    }

    if(!descriptorSetLayoutBindings.empty())
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = descriptorSetLayoutBindings.size();
        layoutInfo.pBindings = descriptorSetLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(
            driver.device, 
            &layoutInfo, 
            nullptr, 
            &descriptorSetLayout_) != VK_SUCCESS) 
        {
            LogError("Failed to create descriptor set layout!");
            return false;
        }
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayoutBindings.empty() ? 0 : 1;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayoutBindings.empty() ? nullptr : &descriptorSetLayout_;
    pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.empty() ? 0 : pushConstantRanges.data();

    if (vkCreatePipelineLayout(
        driver.device, 
        &pipelineLayoutInfo, 
        nullptr,
        &pipelineLayout_) != VK_SUCCESS)
    {
        LogError("Failed to create pipeline layout!\n");
        return false;
    }

    auto renderPass = GetCurrentRenderPass();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    if(pipelinePb.depth_test_enable())
    {
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = pipelinePb.depth_mask() ? VK_TRUE : VK_FALSE;
        switch(pipelinePb.depth_compare_op())
        {
        case core::pb::Pipeline_DepthCompareOp_LESS: 
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            break;
        case core::pb::Pipeline_DepthCompareOp_LESS_OR_EQUAL: 
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            break;
        case core::pb::Pipeline_DepthCompareOp_EQUAL: 
            depthStencil.depthCompareOp = VK_COMPARE_OP_EQUAL;
            break;
        case core::pb::Pipeline_DepthCompareOp_GREATER: 
            depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER;
            break;
        case core::pb::Pipeline_DepthCompareOp_NOT_EQUAL: 
            depthStencil.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
            break;
        case core::pb::Pipeline_DepthCompareOp_GREATER_OR_EQUAL: 
            depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
            break;
        case core::pb::Pipeline_DepthCompareOp_ALWAYS: 
            depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
            break;
        case core::pb::Pipeline_DepthCompareOp_NEVER: 
            depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
            break;
        default:
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            break;
        }
        
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        if(pipelinePb.enable_stencil_test())
        {
            depthStencil.stencilTestEnable = VK_TRUE;
            VkStencilOpState stencilState{};
            stencilState.writeMask = pipelinePb.stencil_mask();
            stencilState.compareMask = pipelinePb.stencil_func_mask();
            stencilState.reference = pipelinePb.stencil_ref();

            switch(pipelinePb.stencil_func())
            {
            case core::pb::Pipeline_StencilFunc_STENCIL_NEVER: 
                stencilState.compareOp = VK_COMPARE_OP_NEVER;
                break;
            case core::pb::Pipeline_StencilFunc_STENCIL_LESS: 
                stencilState.compareOp = VK_COMPARE_OP_LESS;
                break;
            case core::pb::Pipeline_StencilFunc_STENCIL_LEQUAL:
                stencilState.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                break;
            case core::pb::Pipeline_StencilFunc_STENCIL_GREATER:
                stencilState.compareOp = VK_COMPARE_OP_GREATER;
                break;
            case core::pb::Pipeline_StencilFunc_STENCIL_GEQUAL:
                stencilState.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
                break;
            case core::pb::Pipeline_StencilFunc_STENCIL_EQUAL:
                stencilState.compareOp = VK_COMPARE_OP_EQUAL;
                break;
            case core::pb::Pipeline_StencilFunc_STENCIL_NOTEQUAL:
                stencilState.compareOp = VK_COMPARE_OP_NOT_EQUAL;
                break;
            case core::pb::Pipeline_StencilFunc_STENCIL_ALWAYS:
                stencilState.compareOp = VK_COMPARE_OP_ALWAYS;
                break;
            default: 
                break;
            }

            const auto convertStencilOpToVk = [](core::pb::Pipeline_StencilOp stencilOp)
            {
                switch (stencilOp)
                {
                case core::pb::Pipeline_StencilOp_KEEP:
                    return VK_STENCIL_OP_KEEP;
                case core::pb::Pipeline_StencilOp_STENCIL_ZERO:
                    return VK_STENCIL_OP_ZERO;
                case core::pb::Pipeline_StencilOp_REPLACE:
                    return VK_STENCIL_OP_REPLACE;
                case core::pb::Pipeline_StencilOp_INCR:
                    return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
                case core::pb::Pipeline_StencilOp_INCR_WRAP:
                    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
                case core::pb::Pipeline_StencilOp_DECR:
                    return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
                case core::pb::Pipeline_StencilOp_DECR_WRAP:
                    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
                case core::pb::Pipeline_StencilOp_INVERT:
                    return VK_STENCIL_OP_INVERT;
                default:
                    return VK_STENCIL_OP_ZERO;
                }
            };
            stencilState.depthFailOp = convertStencilOpToVk(pipelinePb.stencil_depth_fail());
            stencilState.failOp = convertStencilOpToVk(pipelinePb.stencil_source_fail());
            stencilState.passOp = convertStencilOpToVk(pipelinePb.stencil_depth_pass());

            depthStencil.front = stencilState;
            depthStencil.back = stencilState;
        }
        else
        {
            depthStencil.stencilTestEnable = VK_FALSE;
            depthStencil.front = {}; // Optional
            depthStencil.back = {}; // Optional
        }
        pipelineInfo.pDepthStencilState = &depthStencil;
    }
    else
    {
        pipelineInfo.pDepthStencilState = VK_NULL_HANDLE; 
    }

    std::array dynamicStates =
    {
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
    };

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
    pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
    pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
    pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = nullptr; // &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
    pipelineInfo.layout = pipelineLayout_;
    pipelineInfo.renderPass = renderPass;
    int subpassIndex = -1;
    for(int i = 0; i < sceneInfo.render_pass().sub_passes_size(); i++)
    {
        const auto& subpass = sceneInfo.render_pass().sub_passes(i);
        for(auto& command : subpass.commands())
        {
            if(sceneInfo.materials(command.material_index()).pipeline_index() == pipelineIndex)
            {
                if (subpassIndex != -1 && subpassIndex != i)
                {
                    LogError("Using the pipeline in two subpasses");
                }
                subpassIndex = i;
            }
        }
    }
    if(subpassIndex == -1)
    {
        LogError("No subpass found for the pipeline");
        return false;
    }
    pipelineInfo.subpass = subpassIndex;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional


    if (vkCreateGraphicsPipelines(driver.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
        &pipeline_) != VK_SUCCESS)
    {
        LogError("Failed to create graphics pipeline!\n");
        return false;
    }

    return true;
}

bool Pipeline::LoadComputePipeline(const core::pb::Pipeline& pipelinePb, Shader& computeShader)
{
    pipelineBindPoint_ = VK_PIPELINE_BIND_POINT_COMPUTE;
    return false;
}

bool Pipeline::LoadRaytracingPipeline(const core::pb::RaytracingPipeline& raytracingPipelinePb,
                                      Shader& rayGenShader,
                                      Shader& missHitShader,
                                      Shader& closestHitShader,
                                      std::optional<std::reference_wrapper<Shader>> anyHitShader, 
                                      std::optional<std::reference_wrapper<Shader>> intersectionShadder)
{
    auto& driver = GetDriver();
    pipelineBindPoint_ = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;

    //generate uniform push constant table
    std::vector<std::reference_wrapper<const core::pb::Shader>> shadersInfos;
    std::array shaderIndices =
    {
        raytracingPipelinePb.ray_gen_shader_index(),
        raytracingPipelinePb.miss_hit_shader_index(),
        raytracingPipelinePb.closest_hit_shader_index(),
        raytracingPipelinePb.any_hit_shader_index(),
        raytracingPipelinePb.intersection_hit_shader_index(),
    };
    std::array shaderTypes =
    {
        core::pb::RAY_GEN,
        core::pb::RAY_MISS,
        core::pb::RAY_CLOSEST_HIT,
        core::pb::RAY_ANY_HIT,
        core::pb::RAY_INTERSECTION
    };
    const auto& sceneInfo = core::GetCurrentScene()->GetInfo();
    for (std::size_t i = 0; i < shaderIndices.size(); i++)
    {
        auto shaderIndex = shaderIndices[i];
        if (shaderIndex == -1)
            continue;
        auto& shader = sceneInfo.shaders(shaderIndex);
        if (shader.type() == shaderTypes[i])
        {
            shadersInfos.emplace_back(shader);
        }
    }

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    
    for (auto& shaderInfo : shadersInfos)
    {
        for (int i = 0; i < shaderInfo.get().uniforms_size(); i++)
        {
            VkDescriptorSetLayoutBinding bindingLayout{};
            const auto& uniform = shaderInfo.get().uniforms(i);
            const auto binding = uniform.binding();
            const auto it = std::ranges::find_if(bindings, [binding](const auto& bindingLayout)
                {
                    return bindingLayout.binding == binding;
                });
            if(it != bindings.end())
            {
                it->stageFlags |= GetShaderStage(uniform.stage());
                continue;
            }


            bindingLayout.binding = uniform.binding();
            switch (uniform.type())
            {
            case core::pb::Attribute_Type_ACCELERATION_STRUCT:
                bindingLayout.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                break;
            case core::pb::Attribute_Type_IMAGE2D:
                bindingLayout.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;
            default:
                bindingLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            }
            bindingLayout.descriptorCount = 1;
            bindingLayout.stageFlags = GetShaderStage(uniform.stage());
            bindings.push_back(bindingLayout);
        }
    }
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    if (vkCreateDescriptorSetLayout(
        driver.device,
        &layoutInfo,
        nullptr,
        &descriptorSetLayout_) != VK_SUCCESS)
    {
        LogError("Failed to create descriptor set layout!");
        return false;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount =  1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
    //TODO add push constant
    if (vkCreatePipelineLayout(
        driver.device,
        &pipelineLayoutInfo,
        nullptr,
        &pipelineLayout_) != VK_SUCCESS)
    {
        LogError("Failed to create pipeline layout!\n");
        return false;
    }

    std::vector<std::reference_wrapper<const Shader>> shaders
    {
        rayGenShader,
        missHitShader,
        closestHitShader
    };
    if (anyHitShader)
    {
        shaders.emplace_back(anyHitShader.value());
    }
    if(intersectionShadder)
    {
        shaders.emplace_back(intersectionShadder.value());
    }

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(shaders.size());
    shaderGroups_.reserve(shaders.size());

    for(std::size_t i = 0; i < shaders.size(); i++)
    {
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = shaders[i].get().stage;
        shaderStage.module = shaders[i].get().module;
        shaderStage.pName = "main";
        shaderStages.push_back(shaderStage);

        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        switch(shaders[i].get().stage)
        {
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:
            shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            break;
        default:
            shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            break;
        }
        switch (shaders[i].get().stage)
        {
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
        case VK_SHADER_STAGE_MISS_BIT_KHR:
            shaderGroup.generalShader = i;
            shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
            break;
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.closestHitShader = i;
            shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
            break;
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
            shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.intersectionShader = i;
            break;
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
            shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroup.anyHitShader = i;
            shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
            break;
        default:
            break;
        }
        shaderGroups_.push_back(shaderGroup);
    }

    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
    rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
    rayTracingPipelineCI.pStages = shaderStages.data();
    rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups_.size());
    rayTracingPipelineCI.pGroups = shaderGroups_.data();
    rayTracingPipelineCI.maxPipelineRayRecursionDepth = raytracingPipelinePb.max_recursion_depth();
    rayTracingPipelineCI.layout = pipelineLayout_;
    if(vkCreateRayTracingPipelinesKHR(
        driver.device, 
        VK_NULL_HANDLE, 
        VK_NULL_HANDLE, 
        1, 
        &rayTracingPipelineCI, 
        nullptr,
        &pipeline_) != VK_SUCCESS)
    {
        return false;
    }
    return true;
}

void Pipeline::Bind()
{
    const auto& renderer = GetRenderer();
    vkCmdBindPipeline(renderer.commandBuffers[renderer.imageIndex], pipelineBindPoint_, pipeline_);
}

void Pipeline::Destroy() const
{
    const auto& driver = GetDriver();
    vkDestroyDescriptorSetLayout(driver.device, descriptorSetLayout_, nullptr);
    vkDestroyPipeline(driver.device, pipeline_, nullptr);
    vkDestroyPipelineLayout(driver.device, pipelineLayout_, nullptr);
}

}
