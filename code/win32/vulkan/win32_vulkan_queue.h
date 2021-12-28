/* date = December 20th 2021 4:02 am */

#ifndef WIN32_VULKAN_QFAMILY_H
#define WIN32_VULKAN_QFAMILY_H

internal VkSubmitInfo
vulkan_get_submitinfo(VkSemaphore waitSemaphores[],
                      u32 waitSemaphoreCount,
                      VkPipelineStageFlags waitStages[],
                      VkSemaphore signalSemaphores[],
                      u32 signalSemaphoreCount,
                      VkCommandBuffer *commandBuffers,
                      u32 commandBufferCount)
{
    VkSubmitInfo submitInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = 0,
        .waitSemaphoreCount = waitSemaphoreCount,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = commandBufferCount,
        .pCommandBuffers = commandBuffers,
        .signalSemaphoreCount = signalSemaphoreCount,
        .pSignalSemaphores = signalSemaphores,
    };
    
    return submitInfo;
}

internal VkDeviceQueueCreateInfo
vulkan_get_queue_createinfo(vulkan_context *vk)
{
    // TODO: Leak
    float *queuePriority = (float *)malloc(sizeof(float));
    *queuePriority = 1.0f;
    
    VkDeviceQueueCreateInfo queueCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .queueFamilyIndex = vk->deviceQueueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = queuePriority,
    };
    
    return queueCreateInfo;
}

internal void
vulkan_get_device_queue(vulkan_context *vk)
{
    vkGetDeviceQueue(vk->device, vk->deviceQueueFamilyIndex, 0, &vk->queue);
    
    if (!vk->queue)
    {
        fatal_error("Failed to get device queue!");
    }
}

internal typeless_vector
vulkan_get_device_qfamily_props(VkPhysicalDevice device)
{
    typeless_vector qfamilyProps = {0};
    
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &qfamilyProps.count, 0);
    
    // TODO: Leak
    qfamilyProps.data = (VkQueueFamilyProperties *)
        malloc(qfamilyProps.count * sizeof(VkQueueFamilyProperties));
    
    vkGetPhysicalDeviceQueueFamilyProperties(device, &qfamilyProps.count, qfamilyProps.data);
    
    return qfamilyProps;
}

internal indexed_b32
vulkan_find_suitable_qfamily(vulkan_context *vk, VkPhysicalDevice device,
                             typeless_vector qfamilyProps)
{
    indexed_b32 result = {0};
    for (u32 i = 0;
         i < qfamilyProps.count;
         ++i)
    {
        if (((VkQueueFamilyProperties *)qfamilyProps.data)[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            VkBool32 presentSupport = 0;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vk->surface, &presentSupport);
            
            if (presentSupport)
            {
                result.found = 1;
                result.index = i;
            }
        }
    }
    
    return result;
}

#endif //WIN32_VULKAN_QFAMILY_H
