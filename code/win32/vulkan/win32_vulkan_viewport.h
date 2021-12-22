/* date = December 21st 2021 9:17 am */

#ifndef WIN32_VULKAN_VIEWPORT_H
#define WIN32_VULKAN_VIEWPORT_H

internal VkViewport
vulkan_get_viewport(vulkan_context *vk)
{
    VkViewport viewport = 
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)vk->swapChain.extent.width,
        .height = (float)vk->swapChain.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    
    return viewport;
}

internal VkRect2D
vulkan_get_scissor(vulkan_context *vk)
{
    VkRect2D scissor = 
    {
        .offset = {0, 0},
        .extent = vk->swapChain.extent,
    };
    
    return scissor;
}

internal VkPipelineViewportStateCreateInfo
vulkan_get_viewportstate_createinfo(VkViewport *viewport, VkRect2D *scissor)
{
    VkPipelineViewportStateCreateInfo viewportState = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = viewport,
        .scissorCount = 1,
        .pScissors = scissor,
    };
    
    return viewportState;
}

#endif //WIN32_VULKAN_VIEWPORT_H
