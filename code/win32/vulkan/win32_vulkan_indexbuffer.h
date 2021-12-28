/* date = December 24th 2021 11:04 am */

#ifndef WIN32_VULKAN_INDEXBUFFER_H
#define WIN32_VULKAN_INDEXBUFFER_H

internal void
vulkan_make_indexbuffer(vulkan_context *vk)
{
    VkDeviceSize bufferSize = sizeof(u16) * vk->indices.count;
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vulkan_make_buffer(vk, &stagingBuffer, &stagingBufferMemory, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void* data;
    vkMapMemory(vk->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vk->indices.data, (size_t)bufferSize);
    vkUnmapMemory(vk->device, stagingBufferMemory);
    
    vulkan_make_buffer(vk, &vk->indexBuffer, &vk->indexBufferMemory, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    vulkan_copy_buffer(vk, stagingBuffer, vk->indexBuffer, bufferSize);
    
    vkDestroyBuffer(vk->device, stagingBuffer, 0);
    vkFreeMemory(vk->device, stagingBufferMemory, 0);
}

#endif //WIN32_VULKAN_INDEXBUFFER_H
