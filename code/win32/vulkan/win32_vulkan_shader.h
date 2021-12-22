/* date = December 21st 2021 7:05 am */

#ifndef WIN32_VULKAN_SHADER_H
#define WIN32_VULKAN_SHADER_H

internal VkShaderModuleCreateInfo
vulkan_get_shadermodule_createinfo(binary_file *code)
{
    VkShaderModuleCreateInfo createInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .codeSize = code->byteSize,
        .pCode = (const u32 *)code->data,
    };
    
    return createInfo;
}

internal VkShaderModule
vulkan_make_shader_module(vulkan_context *vk, binary_file *code)
{
    VkShaderModuleCreateInfo createInfo = vulkan_get_shadermodule_createinfo(code);
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(vk->device, &createInfo, 0, &shaderModule) != VK_SUCCESS)
    {
        fatal_error("Failed to create shader module!");
    }
    
    return shaderModule;
}

internal VkPipelineShaderStageCreateInfo
vulkan_get_pipelineshaderstage_createinfo(VkShaderStageFlagBits stage,
                                          VkShaderModule shaderModule)
{
    VkPipelineShaderStageCreateInfo shaderStageInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .stage = stage,
        .module = shaderModule,
        .pName = "main",
        .pSpecializationInfo = 0,
    };
    
    return shaderStageInfo;
}

#endif //WIN32_VULKAN_SHADER_H
