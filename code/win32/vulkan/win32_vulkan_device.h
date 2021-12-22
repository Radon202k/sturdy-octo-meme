/* date = December 20th 2021 4:04 am */

#ifndef WIN32_VULKAN_DEVICE_H
#define WIN32_VULKAN_DEVICE_H

internal VkDeviceCreateInfo
vulkan_get_device_createinfo(vulkan_context *vk,
                             VkDeviceQueueCreateInfo *queueCreateInfo,
                             VkPhysicalDeviceFeatures deviceFeatures)
{
    VkDeviceCreateInfo createInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = queueCreateInfo,
#ifdef VULKAN_USE_VALIDATION_LAYER
        .enabledLayerCount = vk->vLayersNamesArray.count,
        .ppEnabledLayerNames = vk->vLayersNamesArray.data,
#else
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = 0,
#endif
        .enabledExtensionCount = vk->requiredDeviceExtensions.count,
        .ppEnabledExtensionNames = vk->requiredDeviceExtensions.data,
        .pEnabledFeatures = &deviceFeatures,
    };
    
    return createInfo;
}

internal u32
vulkan_rate_device_suitability(VkPhysicalDevice device)
{
    u32 score = 0;
    
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }
    
    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;
    
#if 0
    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader)
    {
        return 0;
    }
#endif
    
    return score;
}

internal typeless_vector
vulkan_query_device_extensions(VkPhysicalDevice device)
{
    typeless_vector result = {0};
    vkEnumerateDeviceExtensionProperties(device, 0, &result.count, 0);
    
    // TODO: Leak
    result.data = (VkExtensionProperties *)malloc(result.count * sizeof(VkExtensionProperties));
    
    vkEnumerateDeviceExtensionProperties(device, 0, &result.count, result.data);
    
    return result;
}

internal string_array
vulkan_get_device_extensions(void)
{
    string_array result = make_string_array(1);
    
    result.data[0] = string_allocate(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    
    return result;
}

internal b32
vulkan_device_supports_extensions(vulkan_context *vk, VkPhysicalDevice device)
{
    typeless_vector deviceExt = vulkan_query_device_extensions(device);
    
    // For each required device extension
    for (u32 j = 0;
         j < vk->requiredDeviceExtensions.count;
         ++j)
    {
        // See if finds it in device extensions
        b32 found = 0;
        for (u32 i = 0;
             i < deviceExt.count;
             ++i)
        {
            // If found it mark the boolean
            if (string_equal(((VkExtensionProperties *)deviceExt.data)[i].extensionName, vk->requiredDeviceExtensions.data[j]))
            {
                found = 1;
            }
        }
        
        // If didn't find it after testing all device extensions
        if (!found)
        {
            // Then device failed the test
            return 0;
        }
    }
    
    // If didn't fail after testing all required extensions
    // Then device supports all extensions
    return 1;
}

internal b32
vulkan_device_suitable(vulkan_context *vk, VkPhysicalDevice device)
{
    if (!vulkan_device_supports_extensions(vk, device))
    {
        return 0;
    }
    
    vulkan_swapchain_support_details swapChainSupport = vulkan_query_swapchain_support_details(vk, device);
    
    if (swapChainSupport.formats.count == 0 ||
        swapChainSupport.presentModes.count == 0)
    {
        return 0;
    }
    
    return 1;
}

internal void
vulkan_pick_physical_device(vulkan_context *vk)
{
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(vk->instance, &deviceCount, 0);
    
    if (deviceCount == 0)
    {
        fatal_error("No vulkan capable devices available. =(");
    }
    
    // TODO: When to free this?
    VkPhysicalDevice *devices = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(vk->instance, &deviceCount, devices);
    
    // Pick first available suitable device
    vk->physicalDevice = VK_NULL_HANDLE;
    for (u32 i = 0;
         i < deviceCount;
         ++i)
    {
        // Get queue family props array for physical device
        typeless_vector props = vulkan_get_device_qfamily_props(devices[i]);
        
        // Check if there is a suitable queue family props in array 
        indexed_b32 r = vulkan_find_suitable_qfamily(vk, devices[i], props);
        
        if (r.found && vulkan_device_suitable(vk, devices[i]))
        {
            // Then save device, qfamily props array and index of suitable one
            vk->physicalDevice = devices[i];
            vk->deviceQueueFamilies = props;
            vk->deviceQueueFamilyIndex = r.index;
            
            // Done searching for physical devices since this is already suitable
            break;
        }
        
        // TODO: free props array here?
    }
    
    if (vk->physicalDevice == VK_NULL_HANDLE)
    {
        fatal_error("No suitable vulkan device was found. =(");
    }
}

internal void
vulkan_make_logical_device(vulkan_context *vk)
{
    VkDeviceQueueCreateInfo queueCreateInfo = vulkan_get_queue_createinfo(vk);
    
    // Specify used physical device features
    VkPhysicalDeviceFeatures deviceFeatures = {0};
    
    VkDeviceCreateInfo createInfo = vulkan_get_device_createinfo(vk, &queueCreateInfo, deviceFeatures);
    
    if (vkCreateDevice(vk->physicalDevice, &createInfo, 0, &vk->device) != VK_SUCCESS)
    {
        fatal_error("Failed to make logical device!");
    }
}

#endif //WIN32_VULKAN_DEVICE_H
