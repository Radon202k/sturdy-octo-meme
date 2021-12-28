/* date = December 21st 2021 4:18 am */

#ifndef WIN32_VULKAN_PIPELINE_H
#define WIN32_VULKAN_PIPELINE_H


internal VkPipelineDynamicStateCreateInfo
vulkan_get_dynamicstate_createinfo(void)
{
    VkDynamicState dynamicStates[] = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates,
    };
    
    return dynamicState;
}

internal VkPipelineLayoutCreateInfo
vulkan_get_pipelinelayout_createinfo(VkDescriptorSetLayout *setLayouts, u32 setLayoutCount)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .setLayoutCount = setLayoutCount,
        .pSetLayouts = setLayouts,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = 0,
    };
    
    return pipelineLayoutInfo;
}

internal VkGraphicsPipelineCreateInfo
vulkan_get_graphicspipeline_createinfo(vulkan_context *vk,
                                       VkPipelineShaderStageCreateInfo shaderStages[],
                                       VkPipelineVertexInputStateCreateInfo *vertexInputInfo,
                                       VkPipelineInputAssemblyStateCreateInfo *inputAssInfo,
                                       VkPipelineViewportStateCreateInfo *viewportState,
                                       VkPipelineRasterizationStateCreateInfo *rasterizerState,
                                       VkPipelineMultisampleStateCreateInfo *multisampleState, 
                                       VkPipelineColorBlendStateCreateInfo *colorBlendState)
{
    VkGraphicsPipelineCreateInfo createInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = vertexInputInfo,
        .pInputAssemblyState = inputAssInfo,
        .pTessellationState = 0,
        .pViewportState = viewportState,
        .pRasterizationState = rasterizerState,
        .pMultisampleState = multisampleState,
        .pDepthStencilState = 0,
        .pColorBlendState = colorBlendState,
        .pDynamicState = 0,
        .layout = vk->graphicsPipelineLayout,
        .renderPass = vk->renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };
    
    return createInfo;
}

internal void
vulkan_make_graphics_pipeline(vulkan_context *vk)
{
    // Shaders
    binary_file vertShaderCode = read_entire_file("shaders/bin/vert.spv");
    binary_file fragShaderCode = read_entire_file("shaders/bin/frag.spv");
    VkShaderModule vertShaderModule = vulkan_make_shader_module(vk, &vertShaderCode);
    VkShaderModule fragShaderModule = vulkan_make_shader_module(vk, &fragShaderCode);
    VkPipelineShaderStageCreateInfo shaderStages[2] = 
    {
        vulkan_get_pipelineshaderstage_createinfo(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule),
        vulkan_get_pipelineshaderstage_createinfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule),
    };
    
    // Input info
    VkVertexInputBindingDescription bindingDesc = vulkan_get_binding_description();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = vulkan_get_vertexinput_createinfo(&bindingDesc);
    VkPipelineInputAssemblyStateCreateInfo inputAssInfo = vulkan_get_inputassembly_createinfo();
    
    // Viewport state
    VkViewport viewport = vulkan_get_viewport(vk);
    VkRect2D scissor = vulkan_get_scissor(vk);
    VkPipelineViewportStateCreateInfo viewportState = vulkan_get_viewportstate_createinfo(&viewport, &scissor);
    
    // Rasterizer info
    VkPipelineRasterizationStateCreateInfo rasterizerInfo = vulkan_get_rasterizationstate_createinfo();
    
    // Multisample state
    VkPipelineMultisampleStateCreateInfo multisampleState = vulkan_get_multisamplestate_createinfo();
    
    // Color blend state
    VkPipelineColorBlendAttachmentState colorBlendAttachment = vulkan_get_colorblend_attachmentstate();
    VkPipelineColorBlendStateCreateInfo colorBlendState = vulkan_get_colorblendstate_createinfo(&colorBlendAttachment);
    
    // Dynamic state
    VkPipelineDynamicStateCreateInfo dynamicState = vulkan_get_dynamicstate_createinfo();
    
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = vulkan_get_pipelinelayout_createinfo(&vk->descriptorSetLayout, 1);
    
    if (vkCreatePipelineLayout(vk->device, &pipelineLayoutInfo, 0, &vk->graphicsPipelineLayout) != VK_SUCCESS)
    {
        fatal_error("Failed to create pipeline layout!");
    }
    
    // Make render pass
    vulkan_make_renderpass(vk);
    
    // Make graphics pipeline
    VkGraphicsPipelineCreateInfo createInfo = 
        vulkan_get_graphicspipeline_createinfo(vk, shaderStages, &vertexInputInfo, &inputAssInfo,
                                               &viewportState, &rasterizerInfo, &multisampleState,
                                               &colorBlendState);
    
    if (vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, &createInfo, 0, &vk->graphicsPipeline) != VK_SUCCESS)
    {
        fatal_error("Failed to create graphics pipeline!");
    }
    
    // Cleanup
    vkDestroyShaderModule(vk->device, fragShaderModule, 0);
    vkDestroyShaderModule(vk->device, vertShaderModule, 0);
}

#endif //WIN32_VULKAN_PIPELINE_H
