/* date = December 19th 2021 0:57 am */

#ifndef WIN32_VULKAN_FUNCTIONS_DEBUG_H
#define WIN32_VULKAN_FUNCTIONS_DEBUG_H

internal vulkan_vlayers
vulkan_query_vlayers(void)
{
    vulkan_vlayers result = {0};
    
    vkEnumerateInstanceLayerProperties(&result.count, 0);
    
    result.props = (VkLayerProperties *)malloc(result.count * sizeof(VkLayerProperties));
    
    vkEnumerateInstanceLayerProperties(&result.count, result.props);
    
    return result;
}

internal b32
vulkan_supports_vlayers(vulkan_context *vk)
{
    for (u32 j = 0;
         j < vk->vLayersNamesArray.count;
         ++j)
    {
        b32 layerFound = 0;
        
        for (u32 i = 0;
             i < vk->vLayers.count;
             ++i)
        {
            VkLayerProperties prop = vk->vLayers.props[i];
            
            if (string_equal(vk->vLayersNamesArray.data[j], prop.layerName))
            {
                layerFound = 1;
                break;
            }
        }
        
        if (!layerFound)
        {
            return 0;
        }
    }
    
    return 1;
}

internal string_array
vulkan_get_vlayers_names_array(void)
{
    string_array result = make_string_array(1);
    
    result.data[0] = string_allocate("VK_LAYER_KHRONOS_validation");
    
    return result;
}

internal VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                     void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        fatal_error(pCallbackData->pMessage);
    }
    else
    {
        show_message(pCallbackData->pMessage);
    }
    
    return VK_FALSE;
}

internal VkDebugUtilsMessengerCreateInfoEXT
vulkan_get_debug_utils_messenger_createinfo(void)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {0};
    
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    
    createInfo.messageSeverity = (VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
    
    createInfo.messageType = (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);
    
    createInfo.pfnUserCallback = vulkan_debugCallback;
    createInfo.pUserData = 0; // Optional
    
    return createInfo;
}

internal void
vulkan_setup_debug_messenger(vulkan_context *vk)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo =
        vulkan_get_debug_utils_messenger_createinfo();
    
    
    if (vk->vkCreateDebugUtilsMessengerEXT(vk->instance, &createInfo, 0,
                                           &vk->debugMessenger) != VK_SUCCESS)
    {
        fatal_error("Failed to set up debug messenger!");
    }
}

internal void
vulkan_init_vlayers(vulkan_context *vk)
{
    // Get array with names of desired vlayers
    vk->vLayersNamesArray = vulkan_get_vlayers_names_array();
    
    // Get available vlayers on selected physical device
    vk->vLayers = vulkan_query_vlayers();
    
    if (!vulkan_supports_vlayers(vk))
    {
        fatal_error("Vulkan doesn't support required validation layers.");
    }
}

#endif //WIN32_VULKAN_FUNCTIONS_DEBUG_H
