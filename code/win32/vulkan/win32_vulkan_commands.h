/* date = December 22nd 2021 7:13 am */

#ifndef WIN32_VULKAN_COMMANDS_H
#define WIN32_VULKAN_COMMANDS_H

internal VkCommandPoolCreateInfo
vulkan_get_commandpool_createinfo(vulkan_context *vk)
{
    VkCommandPoolCreateInfo poolInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .queueFamilyIndex = vk->deviceQueueFamilyIndex,
    };
    
    return poolInfo;
}

internal VkCommandBufferAllocateInfo
vulkan_get_commandbuffer_allocateinfo(vulkan_context *vk)
{
    VkCommandBufferAllocateInfo allocInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = 0,
        .commandPool = vk->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = vk->commandBuffers.count,
    };
    
    return allocInfo;
}

internal VkCommandBufferBeginInfo
vulkan_get_commandbuffer_begininfo(void)
{
    VkCommandBufferBeginInfo beginInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = 0,
        .flags = 0,
        .pInheritanceInfo = 0,
    };
    
    return beginInfo;
}

internal void
vulkan_make_command_pool(vulkan_context *vk)
{
    VkCommandPoolCreateInfo createInfo = vulkan_get_commandpool_createinfo(vk);
    
    if (vkCreateCommandPool(vk->device, &createInfo, 0, &vk->commandPool) != VK_SUCCESS)
    {
        fatal_error("Failed to create command pool!");
    }
}

internal void
vulkan_make_command_buffers(vulkan_context *vk)
{
    u32 count = vk->swapChain.frameBuffers.count;
    vk->commandBuffers = make_typeless_vector(count, sizeof(VkCommandBuffer));
    
    VkCommandBufferAllocateInfo allocInfo = vulkan_get_commandbuffer_allocateinfo(vk);
    
    if (vkAllocateCommandBuffers(vk->device, &allocInfo, vk->commandBuffers.data) != VK_SUCCESS)
    {
        fatal_error("Failed to allocate command buffers!");
    }
    
    for (u32 i = 0;
         i < vk->commandBuffers.count;
         i++) 
    {
        VkFramebuffer frameBuffer = ((VkFramebuffer *)vk->swapChain.frameBuffers.data)[i];
        
        VkCommandBufferBeginInfo beginInfo = vulkan_get_commandbuffer_begininfo();
        VkCommandBuffer commandBuffer = ((VkCommandBuffer *)vk->commandBuffers.data)[i];
        
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            fatal_error("Failed to begin recording command buffer!");
        }
        
        // Populate command buffer with commands
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        VkRenderPassBeginInfo renderPassBeginInfo = vulkan_get_renderpass_begininfo(vk, frameBuffer, &clearColor, 1);
        
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->graphicsPipeline);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer);
        
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            fatal_error("Failed to record command buffer!");
        }
    }
}

#endif //WIN32_VULKAN_COMMANDS_H
