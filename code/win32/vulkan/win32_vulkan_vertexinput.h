/* date = December 21st 2021 9:16 am */

#ifndef WIN32_VULKAN_INPUT_H
#define WIN32_VULKAN_INPUT_H

internal VkVertexInputBindingDescription
vulkan_get_binding_description(void)
{
    VkVertexInputBindingDescription bindingDescription = 
    {
        .binding = 0,
        .stride = sizeof(vulkan_vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    
    return bindingDescription;
}

internal VkVertexInputAttributeDescription
vulkan_get_vertexinputattribute_description(u32 location, VkFormat format, u32 offset)
{
    VkVertexInputAttributeDescription attributeDescription = 
    {
        .binding = 0,
        .location = location,
        .format = format,
        .offset = offset,
    };
    
    return attributeDescription;
}

internal typeless_vector
vulkan_get_vertexattribute_descriptions(void)
{
    typeless_vector attributeDescriptions = make_typeless_vector(2, sizeof(VkVertexInputAttributeDescription));
    
    ((VkVertexInputAttributeDescription *)attributeDescriptions.data)[0] = vulkan_get_vertexinputattribute_description(0, VK_FORMAT_R32G32_SFLOAT, offsetof(vulkan_vertex, pos));
    
    ((VkVertexInputAttributeDescription *)attributeDescriptions.data)[1] = vulkan_get_vertexinputattribute_description(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vulkan_vertex, color));
    
    return attributeDescriptions;
}

internal VkPipelineVertexInputStateCreateInfo
vulkan_get_vertexinput_createinfo(VkVertexInputBindingDescription *bindingDescriptions)
{
    typeless_vector attributeDescriptions = vulkan_get_vertexattribute_descriptions();
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = bindingDescriptions,
        .vertexAttributeDescriptionCount = attributeDescriptions.count,
        .pVertexAttributeDescriptions = attributeDescriptions.data,
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
