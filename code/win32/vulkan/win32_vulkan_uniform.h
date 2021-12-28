/* date = December 28th 2021 3:02 pm */

#ifndef WIN32_VULKAN_UNIFORM_H
#define WIN32_VULKAN_UNIFORM_H

internal void
vulkan_make_uniform_buffers(vulkan_context *vk)
{
    VkDeviceSize bufferSize = sizeof(vulkan_ubo);
    u32 count = vk->swapChain.images.count;
    vk->uniformBuffers = make_typeless_vector(count, sizeof(VkBuffer));
    vk->uniformBuffersMemory = make_typeless_vector(count, sizeof(VkDeviceMemory));
    for (u32 i = 0;
         i < count;
         ++i)
    {
        VkBuffer *buffer = &((VkBuffer *)vk->uniformBuffers.data)[i];
        VkDeviceMemory *bufferMemory = &((VkDeviceMemory *)vk->uniformBuffersMemory.data)[i];
        
        vulkan_make_buffer(vk, buffer, bufferMemory, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
    }
}

internal void
vulkan_update_uniform_data(vulkan_context *vk, u32 imageIndex)
{
    static f32 time = 0.0f;
    time += 0.01f;
    f32 ar = (f32)vk->swapChain.extent.height/(f32)vk->swapChain.extent.width;
    
#if 1
    vulkan_ubo ubo = 
    {
        .model = mat4_get_rotate_z(time),
        .view = mat4_lookat(V3(0, 2, 2), V3(0, 0, 0)),
        .proj = mat4_perspective(ar, 0.6f, 0.1f, 100.0f).forward,
        // .proj = mat4_identity(1.0f),
    };
    
    
#else
    vulkan_ubo ubo = 
    {
        .model = mat4_identity(1.0f),
        .view = mat4_identity(1.0f),
        .proj = mat4_identity(1.0f),
    };
#endif
    
    
    void *data;
    VkDeviceMemory uboMemory = ((VkDeviceMemory *)vk->uniformBuffersMemory.data)[imageIndex];
    
    vkMapMemory(vk->device, uboMemory, 0, sizeof(ubo), 0, &data);
    
    memcpy(data, &ubo, sizeof(ubo));
    
    vkUnmapMemory(vk->device, uboMemory);
}

#endif //WIN32_VULKAN_UNIFORM_H
