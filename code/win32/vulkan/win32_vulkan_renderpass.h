/* date = December 21st 2021 9:21 am */

#ifndef WIN32_VULKAN_RENDERPASS_H
#define WIN32_VULKAN_RENDERPASS_H

internal VkAttachmentDescription
vulkan_get_attachment_desc(vulkan_context *vk)
{
    VkAttachmentDescription attachment = 
    {
        .flags = 0,
        .format = vk->swapChain.imageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // Image to be presented to swapchain
    };
    
    return attachment;
}

internal VkAttachmentReference
vulkan_get_attachment_reference(u32 attachment, VkImageLayout layout)
{
    VkAttachmentReference attachmentRef = 
    {
        .attachment = attachment,
        .layout = layout,
    };
    
    return attachmentRef;
}

internal VkSubpassDescription
vulkan_get_subpass_desc(VkAttachmentReference *colorAttachmentRef)
{
    VkSubpassDescription subpass = 
    {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = colorAttachmentRef,
        .pResolveAttachments = 0,
        .pDepthStencilAttachment = 0,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = 0,
    };
    
    return subpass;
}

internal VkRenderPassCreateInfo
vulkan_get_renderpass_createinfo(VkAttachmentDescription *colorAttachment,
                                 VkSubpassDescription *subpass,
                                 VkSubpassDependency *dependencies)
{
    VkRenderPassCreateInfo renderPassInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = colorAttachment,
        .subpassCount = 1,
        .pSubpasses = subpass,
        .dependencyCount = 1,
        .pDependencies = dependencies,
    };
    
    return renderPassInfo;
}

internal VkSubpassDependency
vulkan_get_subpass_dependency(void)
{
    VkSubpassDependency dependency = 
    {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0,
    };
    
    return dependency;
}

internal void
vulkan_make_renderpass(vulkan_context *vk)
{
    VkAttachmentDescription colorAttachment = vulkan_get_attachment_desc(vk);
    
    VkAttachmentReference colorAttachmentRef = 
        vulkan_get_attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    
    VkSubpassDescription subpass = vulkan_get_subpass_desc(&colorAttachmentRef);
    
    VkSubpassDependency dependency = vulkan_get_subpass_dependency();
    
    VkRenderPassCreateInfo createInfo = 
        vulkan_get_renderpass_createinfo(&colorAttachment, &subpass, &dependency);
    
    if (vkCreateRenderPass(vk->device, &createInfo, 0, &vk->renderPass) != VK_SUCCESS)
    {
        fatal_error("Failed to create render pass!");
    }
    
}

internal VkRenderPassBeginInfo
vulkan_get_renderpass_begininfo(vulkan_context *vk, VkFramebuffer frameBuffer,
                                VkClearValue *clearColors, u32 clearColorCount)
{
    VkRenderPassBeginInfo renderPassInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = 0,
        .renderPass = vk->renderPass,
        .framebuffer = frameBuffer,
        .renderArea.offset = {0, 0},
        .renderArea.extent = vk->swapChain.extent,
        .clearValueCount = clearColorCount,
        .pClearValues = clearColors,
    };
    
    return renderPassInfo;
}

#endif //WIN32_VULKAN_RENDERPASS_H
