#include "vk/pipeline.h"

#include "vk/engine.h"
#include "vk/scene.h"

namespace gpr5300::vk
{
bool Pipeline::LoadRaterizePipeline(const pb::Pipeline& pipelinePb, Shader& vertexShader, Shader& fragmentShader)
{
    auto& swapchain = GetSwapchain();
    auto& driver = GetDriver();
    std::array<VkShaderModule, 2> shaderModules{ {vertexShader.module, fragmentShader.module} };
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
        {
            {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage = vertexShader.stage, .module = vertexShader.module,.pName = "main"},
            {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,.stage = fragmentShader.stage, .module = fragmentShader.module,.pName = "main"}
        } };
    std::vector<VkVertexInputBindingDescription> vertexBindingDescriptors;
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptors;
    //TODO fill binding and attribute descriptors

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = vertexBindingDescriptors.size();
    vertexInputInfo.pVertexBindingDescriptions = vertexBindingDescriptors.empty() ? nullptr : vertexBindingDescriptors.data();
    vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptors.size();
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptors.empty() ? nullptr : vertexAttributeDescriptors.data(); // Optional

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
    switch (pipelinePb.cull_face())
    {
    case pb::Pipeline_CullFace_BACK:

        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        break;
    case pb::Pipeline_CullFace_FRONT:

        rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
        break;
    case pb::Pipeline_CullFace_FRONT_AND_BACK:

        rasterizer.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
        break;
    default: break;
    }

    switch (pipelinePb.front_face())
    {
    case pb::Pipeline_FrontFace_CLOCKWISE:
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        break;
    case pb::Pipeline_FrontFace_COUNTER_CLOCKWISE:
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

    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(driver.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS)
    {
        LogError("Failed to create pipeline layout!\n");
        return false;
    }

    auto renderPass = GetCurrentRenderPass();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    //TODO manage depth stencil if any
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    if(pipelinePb.depth_test_enable())
    {
        
    }
    else
    {
        pipelineInfo.pDepthStencilState = VK_NULL_HANDLE; // Optional
    }
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional


    if (vkCreateGraphicsPipelines(driver.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
        &pipeline) != VK_SUCCESS)
    {
        LogError("Failed to create graphics pipeline!\n");
        return false;
    }

    return true;
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

void Pipeline::Destroy() const
{
    const auto& driver = GetDriver();
    vkDestroyPipeline(driver.device, pipeline, nullptr);
    vkDestroyPipelineLayout(driver.device, pipelineLayout, nullptr);
}
}
