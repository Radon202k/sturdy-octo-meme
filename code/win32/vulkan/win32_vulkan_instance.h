/* date = December 20th 2021 4:01 am */

#ifndef WIN32_VULKAN_INSTANCE_H
#define WIN32_VULKAN_INSTANCE_H

internal VkInstanceCreateInfo
vulkan_get_instance_createinfo(vulkan_context *vk, VkApplicationInfo *appInfo)
{
    VkInstanceCreateInfo createInfo = {0};
    
    VkInstanceCreateInfo temp =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .pApplicationInfo = appInfo,
        .enabledExtensionCount = vk->requiredInstanceExtensions.count,
        .ppEnabledExtensionNames = vk->requiredInstanceExtensions.data,
        
#ifdef VULKAN_USE_VALIDATION_LAYER
        .enabledLayerCount = vk->vLayersNamesArray.count,
        .ppEnabledLayerNames = vk->vLayersNamesArray.data,
#else
        .enabledLayerCount = 0,
#endif
    };
    
    createInfo = temp;
    
    return createInfo;
}

internal typeless_vector
vulkan_query_instance_extensions(void)
{
    typeless_vector result = {0};
    
    vkEnumerateInstanceExtensionProperties(0, &result.count, 0);
    
    // TODO: Leak
    result.data = (VkExtensionProperties *)malloc(result.count * sizeof(VkExtensionProperties));
    
    vkEnumerateInstanceExtensionProperties(0, &result.count, result.data);
    
    return result;
}

internal string_array
vulkan_get_instance_extensions(void)
{
#ifdef VULKAN_USE_VALIDATION_LAYER
    string_array result = make_string_array(3);
#else
    string_array result = make_string_array(2);
#endif
    
    result.data[0] = string_allocate(VK_KHR_SURFACE_EXTENSION_NAME);
    result.data[1] = string_allocate(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    
#ifdef VULKAN_USE_VALIDATION_LAYER
    result.data[2] = string_allocate(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    
    return result;
}

internal void
vulkan_pre_instance_config(vulkan_context *vk)
{
    vk->requiredInstanceExtensions = vulkan_get_instance_extensions();
    vk->requiredDeviceExtensions = vulkan_get_device_extensions();
    
#ifdef VULKAN_USE_VALIDATION_LAYER
    vulkan_init_vlayers(vk);
#endif
}

internal void
vulkan_make_instance(vulkan_context *vk)
{
    VkApplicationInfo appInfo = vulkan_get_applicationinfo();
    VkInstanceCreateInfo createInfo = vulkan_get_instance_createinfo(vk, &appInfo);
    
#ifdef VULKAN_USE_VALIDATION_LAYER
    // Somehow providing a debug create info (dci) in the pNext field of createInfo
    // allows vulkan to detect warnings/errors when creating/destroying the vk instance
    VkDebugUtilsMessengerCreateInfoEXT dci = vulkan_get_debug_utils_messenger_createinfo();
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&dci;
#endif
    
    if (vkCreateInstance(&createInfo, 0, &vk->instance) != VK_SUCCESS)
    {
        fatal_error("Failed to create vulkan instance!");
    }
}

#endif //WIN32_VULKAN_INSTANCE_H
