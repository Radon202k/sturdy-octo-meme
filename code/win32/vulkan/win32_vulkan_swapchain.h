/* date = December 20th 2021 4:21 am */

#ifndef WIN32_VULKAN_SWAPCHAIN_H
#define WIN32_VULKAN_SWAPCHAIN_H

internal typeless_vector
vulkan_get_physical_device_surface_formats_khr(VkPhysicalDevice device,
                                               VkSurfaceKHR surface)
{
    typeless_vector result = {0};
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &result.count, 0);
    
    if (result.count != 0)
    {
        result.data = (VkSurfaceFormatKHR *)malloc(result.count * sizeof(VkSurfaceFormatKHR));
        
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &result.count, result.data);
    }
    
    return result;
}

internal typeless_vector
vulkan_get_physical_device_surface_presentmodes(VkPhysicalDevice device,
                                                VkSurfaceKHR surface)
{
    typeless_vector result = {0};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &result.count, 0);
    
    if (result.count != 0)
    {
        result.data = (VkPresentModeKHR *)malloc(result.count * sizeof(VkPresentModeKHR));
        
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &result.count, result.data);
    }
    
    return result;
}

internal vulkan_swapchain_support_details
vulkan_query_swapchain_support_details(vulkan_context *vk, VkPhysicalDevice device)
{
    vulkan_swapchain_support_details result = {0};
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vk->surface, &result.capabilities);
    
    result.formats = vulkan_get_physical_device_surface_formats_khr(device, vk->surface);
    
    result.presentModes = vulkan_get_physical_device_surface_presentmodes(device, vk->surface);
    
    return result;
}

internal VkSurfaceFormatKHR
vulkan_choose_swap_surface_format(typeless_vector availableFormats)
{
    for (u32 i = 0;
         i < availableFormats.count;
         ++i)
    {
        VkSurfaceFormatKHR availableFormat = ((VkSurfaceFormatKHR *)availableFormats.data)[i];
        
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    
    VkSurfaceFormatKHR firstAvailableFormat = ((VkSurfaceFormatKHR *)availableFormats.data)[0];
    
    return firstAvailableFormat;
}

internal VkPresentModeKHR
vulkan_choose_swap_present_mode(typeless_vector availablePresentModes)
{
    // See if tripple buffering is available and then choose it
    for (u32 i = 0;
         i < availablePresentModes.count;
         ++i)
    {
        VkPresentModeKHR availablePresentMode = ((VkPresentModeKHR *)availablePresentModes.data)[i];
        
        // TODO: Should we really use tripple buffering?
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

internal VkExtent2D
vulkan_choose_swap_extent(vulkan_context *vk, VkSurfaceCapabilitiesKHR *capabilities)
{
    if (capabilities->currentExtent.width != UINT32_MAX)
    {
        return capabilities->currentExtent;
    }
    else
    {
        // TODO: High dpi displays
        
        VkExtent2D actualExtent =
        {
            .width = vk->windowWidth,
            .height = vk->windowHeight,
        };
        
        actualExtent.width = clamp_u32(actualExtent.width,
                                       capabilities->minImageExtent.width,
                                       capabilities->maxImageExtent.width);
        
        actualExtent.height = clamp_u32(actualExtent.height, 
                                        capabilities->minImageExtent.height,
                                        capabilities->maxImageExtent.height);
        
        return actualExtent;
    }
}

internal VkSwapchainCreateInfoKHR
vulkan_get_swapchain_createinfo_khr(vulkan_context *vk, 
                                    vulkan_swapchain_support_details *swapChainSupport,
                                    VkSurfaceFormatKHR *surfaceFormat,
                                    VkPresentModeKHR *presentMode,
                                    VkExtent2D *extent,
                                    u32 imageCount)
{
    VkSwapchainCreateInfoKHR createInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vk->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat->format,
        .imageColorSpace = surfaceFormat->colorSpace,
        .imageExtent = *extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        // TODO: switch to VK_IMAGE_USAGE_TRANSFER_DST_BIT
        // for rendering to a different image then copying
        // its contents to the swapchain image. 
        // e.g. post processing
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = 0,
        .preTransform = swapChainSupport->capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = *presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    
    return createInfo;
}

internal VkImageViewCreateInfo
vulkan_get_imageview_createinfo(VkImage image, VkFormat imageFormat)
{
    VkImageViewCreateInfo createInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = imageFormat,
        .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
        .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };
    
    return createInfo;
}

internal void
vulkan_make_swapchain_imageviews(vulkan_context *vk)
{
    vk->swapChain.imageViews = make_typeless_vector(vk->swapChain.images.count,
                                                    sizeof(VkImageView));
    
    for (u32 i = 0;
         i < vk->swapChain.images.count;
         i++)
    {
        VkImageViewCreateInfo createInfo = 
            vulkan_get_imageview_createinfo(((VkImage *)vk->swapChain.images.data)[i],
                                            vk->swapChain.imageFormat);
        
        if (vkCreateImageView(vk->device, &createInfo, 0,
                              &((VkImageView *)vk->swapChain.imageViews.data)[i]) != VK_SUCCESS)
        {
            fatal_error("Failed to create image views!");
        }
    }
}

internal void
vulkan_make_swapchain(vulkan_context *vk)
{
    // Query data
    vulkan_swapchain_support_details swapChainSupport = vulkan_query_swapchain_support_details(vk, vk->physicalDevice);
    
    VkSurfaceFormatKHR surfaceFormat = vulkan_choose_swap_surface_format(swapChainSupport.formats);
    
    VkPresentModeKHR presentMode = vulkan_choose_swap_present_mode(swapChainSupport.presentModes);
    
    VkExtent2D extent = vulkan_choose_swap_extent(vk, &swapChainSupport.capabilities);
    
    u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
    
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    // Create swap chain
    VkSwapchainCreateInfoKHR createInfo = 
        vulkan_get_swapchain_createinfo_khr(vk, &swapChainSupport, 
                                            &surfaceFormat,
                                            &presentMode,
                                            &extent, 
                                            imageCount);
    if (vkCreateSwapchainKHR(vk->device, &createInfo, 0, &vk->swapChain.object) != VK_SUCCESS)
    {
        fatal_error("Failed to create swap chain!");
    }
    
    // Get handles to images
    vkGetSwapchainImagesKHR(vk->device, vk->swapChain.object, &imageCount, 0);
    
    vk->swapChain.images = make_typeless_vector(imageCount, sizeof(VkImage));
    
    vkGetSwapchainImagesKHR(vk->device, vk->swapChain.object, &imageCount, vk->swapChain.images.data);
    
    // Save chosen format and extent
    vk->swapChain.imageFormat = surfaceFormat.format;
    vk->swapChain.extent = extent;
    
    vulkan_make_swapchain_imageviews(vk);
}

internal VkFramebufferCreateInfo
vulkan_get_framebuffer_createinfo(vulkan_context *vk, VkImageView attachments[])
{
    VkFramebufferCreateInfo framebufferInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .renderPass = vk->renderPass,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .width = vk->swapChain.extent.width,
        .height = vk->swapChain.extent.height,
        .layers = 1,
    };
    
    return framebufferInfo;
}

internal void
vulkan_make_swapchain_framebuffers(vulkan_context *vk)
{
    u32 count = vk->swapChain.imageViews.count;
    vk->swapChain.frameBuffers = make_typeless_vector(count, sizeof(VkFramebuffer));
    
    for (u32 i = 0;
         i < vk->swapChain.imageViews.count;
         i++)
    {
        VkImageView attachments[] =
        {
            ((VkImageView *)vk->swapChain.imageViews.data)[i]
        };
        
        VkFramebufferCreateInfo createInfo = vulkan_get_framebuffer_createinfo(vk, attachments);
        
        VkFramebuffer *frameBuffer = &((VkFramebuffer *)vk->swapChain.frameBuffers.data)[i];
        if (vkCreateFramebuffer(vk->device, &createInfo, 0, frameBuffer) != VK_SUCCESS)
        {
            fatal_error("Failed to create framebuffer!");
        }
    }
}

internal VkSemaphoreCreateInfo
vulkan_get_semaphore_createinfo(void)
{
    VkSemaphoreCreateInfo semaphoreInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
    };
    
    return semaphoreInfo;
}

internal VkFenceCreateInfo
vulkan_get_fence_createinfo(void)
{
    VkFenceCreateInfo fenceInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = 0,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    
    return fenceInfo;
}

internal void
vulkan_make_swapchain_sync_objects(vulkan_context *vk)
{
    VkSemaphoreCreateInfo createInfo = vulkan_get_semaphore_createinfo();
    
    vk->swapChain.imageAvailableSemaphores = 
        make_typeless_vector(VULKAN_MAX_FRAMES_IN_FLIGHT, sizeof(VkSemaphore));
    
    vk->swapChain.renderFinishedSemaphores = 
        make_typeless_vector(VULKAN_MAX_FRAMES_IN_FLIGHT, sizeof(VkSemaphore));
    
    vk->swapChain.inFlightFences = 
        make_typeless_vector(VULKAN_MAX_FRAMES_IN_FLIGHT, sizeof(VkFence));
    
    vk->swapChain.imagesInFlight = 
        make_typeless_vector(vk->swapChain.images.count, sizeof(VkFence));
    
    // Initialize imagesinflight with null value
    for (u32 i = 0;
         i < vk->swapChain.images.count;
         ++i)
    {
        ((VkFence *)vk->swapChain.imagesInFlight.data)[i] = VK_NULL_HANDLE;
    }
    
    VkFenceCreateInfo fenceInfo = vulkan_get_fence_createinfo();
    
    for (u32 i = 0;
         i < VULKAN_MAX_FRAMES_IN_FLIGHT;
         ++i)
    {
        if (vkCreateSemaphore(vk->device, &createInfo, 0, &((VkSemaphore *)vk->swapChain.imageAvailableSemaphores.data)[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vk->device, &createInfo, 0, &((VkSemaphore *)vk->swapChain.renderFinishedSemaphores.data)[i]) != VK_SUCCESS ||
            vkCreateFence(vk->device, &fenceInfo, 0, &((VkFence *)vk->swapChain.inFlightFences.data)[i]) != VK_SUCCESS)
        {
            fatal_error("Failed to create semaphores!");
        }
    }
}

#endif //WIN32_VULKAN_SWAPCHAIN_H
