/* date = December 23rd 2021 10:06 am */

#ifndef WIN32_VULKAN_VERTEXBUFFER_H
#define WIN32_VULKAN_VERTEXBUFFER_H

internal void
vulkan_make_vertexbuffer(vulkan_context *vk)
{
    VkDeviceSize bufferSize = sizeof(vulkan_vertex) * vk->vertices.count;
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vulkan_make_buffer(vk, &stagingBuffer, &stagingBufferMemory, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void* data;
    vkMapMemory(vk->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vk->vertices.data, (size_t)bufferSize);
    vkUnmapMemory(vk->device, stagingBufferMemory);
    
    vulkan_make_buffer(vk, &vk->vertexBuffer, &vk->vertexBufferMemory, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    vulkan_copy_buffer(vk, stagingBuffer, vk->vertexBuffer, bufferSize);
    
    vkDestroyBuffer(vk->device, stagingBuffer, 0);
    vkFreeMemory(vk->device, stagingBufferMemory, 0);
}

#endif //WIN32_VULKAN_VERTEXBUFFER_H
