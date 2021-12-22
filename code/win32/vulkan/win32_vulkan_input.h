/* date = December 21st 2021 9:16 am */

#ifndef WIN32_VULKAN_INPUT_H
#define WIN32_VULKAN_INPUT_H

internal VkPipelineVertexInputStateCreateInfo
vulkan_get_vertexinput_createinfo(void)
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = 0,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = 0,
    };
    
    return vertexInputInfo;
}

internal VkPipelineInputAssemblyStateCreateInfo
vulkan_get_inputassembly_createinfo(void)
{
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
    
    return inputAssembly;
}

#endif //WIN32_VULKAN_INPUT_H
