/* date = December 28th 2021 3:06 pm */

#ifndef WIN32_VULKAN_BUFFER_H
#define WIN32_VULKAN_BUFFER_H

internal VkBufferCreateInfo
vulkan_get_buffer_createinfo(vulkan_context *vk, VkDeviceSize bufferSize, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo bufferInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .size = bufferSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = 0,
    };
    
    return bufferInfo;
}


internal u32
vulkan_vertexbuffer_find_memory_type(vulkan_context *vk, u32 typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vk->physicalDevice, &memProperties);
    
    for (u32 i = 0;
         i < memProperties.memoryTypeCount;
         ++i)
    {
        if ((typeFilter & (1 << i)) && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties))
        {
            return i;
        }
    }
    
    fatal_error("Failed to find suitable memory type!");
    
    return 0;
}

internal VkMemoryAllocateInfo
vulkan_get_buffermemory_allocinfo(vulkan_context *vk, VkMemoryRequirements memRequirements)
{
    VkMemoryAllocateInfo allocInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = 0,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vulkan_vertexbuffer_find_memory_type(vk, 
                                                                memRequirements.memoryTypeBits, 
                                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|
                                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
    };
    
    return allocInfo;
}

internal void
vulkan_make_buffer(vulkan_context *vk,
                   VkBuffer *buffer, 
                   VkDeviceMemory *bufferMemory, 
                   VkDeviceSize size, 
                   VkBufferUsageFlags usage,
                   VkMemoryPropertyFlags properties)
{
    VkBufferCreateInfo bufferInfo = vulkan_get_buffer_createinfo(vk, size, usage);
    
    if (vkCreateBuffer(vk->device, &bufferInfo, 0, buffer) != VK_SUCCESS) 
    {
        fatal_error("Failed to create vertex buffer!");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vk->device, *buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = vulkan_get_buffermemory_allocinfo(vk, memRequirements);
    
    if (vkAllocateMemory(vk->device, &allocInfo, 0, bufferMemory) != VK_SUCCESS)
    {
        fatal_error("Failed to allocate vertex buffer memory!");
    }
    
    vkBindBufferMemory(vk->device, *buffer, *bufferMemory, 0);
}

internal VkBufferCopy
vulkan_get_buffer_copy(VkDeviceSize src, VkDeviceSize dst, VkDeviceSize size)
{
    VkBufferCopy copyRegion = 
    {
        .srcOffset = src,
        .dstOffset = dst,
        .size = size,
    };
    
    return copyRegion;
}

internal void
vulkan_copy_buffer(vulkan_context *vk, 
                   VkBuffer srcBuffer, 
                   VkBuffer dstBuffer, 
                   VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = vulkan_get_commandbuffer_allocateinfo(vk, 1);
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vk->device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo = vulkan_get_commandbuffer_begininfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    VkBufferCopy copyRegion = vulkan_get_buffer_copy(0, 0, size);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo = vulkan_get_submitinfo(0, 0, 0, 0, 0, &commandBuffer, 1);
    
    vkQueueSubmit(vk->queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vk->queue);
    vkFreeCommandBuffers(vk->device, vk->commandPool, 1, &commandBuffer);
}


#endif //WIN32_VULKAN_BUFFER_H
