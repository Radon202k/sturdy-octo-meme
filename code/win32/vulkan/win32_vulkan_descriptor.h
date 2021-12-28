/* date = December 28th 2021 5:27 pm */

#ifndef WIN32_VULKAN_DESCRIPTOR_H
#define WIN32_VULKAN_DESCRIPTOR_H

internal VkDescriptorSetLayoutBinding
vulkan_get_descriptorset_layoutbinding(u32 binding, 
                                       VkDescriptorType type,
                                       u32 count,
                                       VkShaderStageFlags stageFlags)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = 
    {
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = count,
        .stageFlags = stageFlags,
        .pImmutableSamplers = 0,
    };
    
    return uboLayoutBinding;
}

internal VkDescriptorSetLayoutCreateInfo
vulkan_get_descriptor_setlayout_createinfo(VkDescriptorSetLayoutBinding *uboLayoutBinding)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .bindingCount = 1,
        .pBindings = uboLayoutBinding,
    };
    
    return layoutInfo;
}

internal void
vulkan_make_descriptor_set_layout(vulkan_context *vk)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = vulkan_get_descriptorset_layoutbinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    
    VkDescriptorSetLayoutCreateInfo layoutInfo = vulkan_get_descriptor_setlayout_createinfo(&uboLayoutBinding);
    
    if (vkCreateDescriptorSetLayout(vk->device, &layoutInfo, 0, &vk->descriptorSetLayout) != VK_SUCCESS)
    {
        fatal_error("Failed to create descriptor set layout!");
    }
}

internal VkDescriptorPoolSize
vulkan_get_descriptorpool_size(VkDescriptorType type, u32 count)
{
    VkDescriptorPoolSize poolSize = 
    {
        .type = type,
        .descriptorCount = count,
    };
    
    return poolSize;
}

internal VkDescriptorPoolCreateInfo
vulkan_get_descriptorpool_createinfo(u32 count, u32 maxSets, VkDescriptorPoolSize *poolSizes)
{
    VkDescriptorPoolCreateInfo poolInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .maxSets = maxSets,
        .poolSizeCount = count,
        .pPoolSizes = poolSizes,
    };
    
    return poolInfo;
}

internal void
vulkan_make_descriptor_pool(vulkan_context *vk)
{
    u32 count = vk->swapChain.images.count;
    
    VkDescriptorPoolSize poolSize = vulkan_get_descriptorpool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, count);
    
    VkDescriptorPoolCreateInfo createInfo = 
        vulkan_get_descriptorpool_createinfo(1, count, &poolSize);
    
    if (vkCreateDescriptorPool(vk->device, &createInfo, 0, &vk->descriptorPool) != VK_SUCCESS)
    {
        fatal_error("failed to create descriptor pool!");
    }
}

internal VkDescriptorSetAllocateInfo
vulkan_get_descriptorset_allocinfo(VkDescriptorPool descriptorPool, u32 count, VkDescriptorSetLayout *layouts)
{
    VkDescriptorSetAllocateInfo allocInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = 0,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = count,
        .pSetLayouts = layouts,
    };
    
    return allocInfo;
}

internal VkDescriptorBufferInfo
vulkan_get_descriptor_bufferinfo(VkBuffer buffer, VkDeviceSize range)
{
    VkDescriptorBufferInfo bufferInfo = 
    {
        .buffer = buffer,
        .offset = 0,
        .range = range,
    };
    
    return bufferInfo;
}

internal VkWriteDescriptorSet
vulkan_get_writedescriptorset(VkDescriptorSet descriptorSet,
                              VkDescriptorType type,
                              u32 count,
                              VkDescriptorBufferInfo *bufferInfo)
{
    VkWriteDescriptorSet descriptorWrite = 
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = 0,
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = count,
        .descriptorType = type,
        .pImageInfo = 0,
        .pBufferInfo = bufferInfo,
        .pTexelBufferView = 0,
    };
    
    return descriptorWrite;
}

internal void
vulkan_make_descriptor_sets(vulkan_context *vk)
{
    u32 count = vk->swapChain.images.count;
    
    typeless_vector layouts = make_typeless_vector(count, sizeof(VkDescriptorSetLayout));
    
    for (u32 i = 0;
         i < count;
         ++i)
    {
        ((VkDescriptorSetLayout *)layouts.data)[i] = vk->descriptorSetLayout;
    }
    
    VkDescriptorSetAllocateInfo allocInfo = 
        vulkan_get_descriptorset_allocinfo(vk->descriptorPool, count, layouts.data);
    
    vk->descriptorSets = make_typeless_vector(count, sizeof(VkDescriptorSet));
    if (vkAllocateDescriptorSets(vk->device, &allocInfo, vk->descriptorSets.data) != VK_SUCCESS)
    {
        fatal_error("Failed to allocate descriptor sets!");
    }
    
    for (u32 i = 0;
         i < count;
         ++i)
    {
        VkBuffer buffer = ((VkBuffer *)vk->uniformBuffers.data)[i];
        
        VkDescriptorBufferInfo bufferInfo =
            vulkan_get_descriptor_bufferinfo(buffer, sizeof(vulkan_ubo));
        
        VkDescriptorSet descriptorSet = ((VkDescriptorSet *)vk->descriptorSets.data)[i];
        
        VkWriteDescriptorSet descriptorWrite = 
            vulkan_get_writedescriptorset(descriptorSet,
                                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                          1,
                                          &bufferInfo);
        
        
        vkUpdateDescriptorSets(vk->device, 1, &descriptorWrite, 0, 0);
    }
    
    
}


#endif //WIN32_VULKAN_DESCRIPTOR_H
