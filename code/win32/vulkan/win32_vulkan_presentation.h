/* date = December 22nd 2021 9:49 am */

#ifndef WIN32_VULKAN_PRESENTATION_H
#define WIN32_VULKAN_PRESENTATION_H

internal VkSubmitInfo
vulkan_get_submitinfo(VkSemaphore waitSemaphores[], 
                      VkPipelineStageFlags waitStages[],
                      VkSemaphore signalSemaphores[],
                      VkCommandBuffer *commandBuffers)
{
    VkSubmitInfo submitInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = 0,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = commandBuffers,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };
    
    return submitInfo;
}

internal VkPresentInfoKHR
vulkan_get_presentinfo_khr(VkSemaphore signalSemaphores[],
                           VkSwapchainKHR swapChains[],
                           u32 *imageIndices)
{
    VkPresentInfoKHR presentInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = 0,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = imageIndices,
        .pResults = 0,
    };
    
    return presentInfo;
}
internal void
vulkan_cleanup_swapchain(vulkan_context *vk)
{
    for (u32 i = 0;
         i < vk->swapChain.frameBuffers.count;
         ++i)
    {
        VkFramebuffer frameBuffer = ((VkFramebuffer *)vk->swapChain.frameBuffers.data)[i];
        vkDestroyFramebuffer(vk->device, frameBuffer, 0);
    }
    
    vkFreeCommandBuffers(vk->device, vk->commandPool, vk->commandBuffers.count, vk->commandBuffers.data);
    
    vkDestroyPipeline(vk->device, vk->graphicsPipeline, 0);
    vkDestroyPipelineLayout(vk->device, vk->graphicsPipelineLayout, 0);
    vkDestroyRenderPass(vk->device, vk->renderPass, 0);
    
    for (u32 i = 0;
         i < vk->swapChain.imageViews.count;
         ++i)
    {
        VkImageView imageView = ((VkImageView *)vk->swapChain.imageViews.data)[i];
        vkDestroyImageView(vk->device, imageView, 0);
    }
    
    vkDestroySwapchainKHR(vk->device, vk->swapChain.object, 0);
}

internal void
vulkan_remake_swapchain(vulkan_context *vk)
{
    window_size windowSize = win32_get_window_size(vk->hwnd);
    while (windowSize.width == 0 || windowSize.height == 0)
    {
        windowSize = win32_get_window_size(vk->hwnd);
        
        MSG msg;
        while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        
        Sleep(100);
    }
    
    vkDeviceWaitIdle(vk->device);
    
    vulkan_cleanup_swapchain(vk);
    
    vulkan_make_swapchain(vk);
    vulkan_make_swapchain_imageviews(vk);
    vulkan_make_renderpass(vk);
    vulkan_make_graphics_pipeline(vk);
    vulkan_make_swapchain_framebuffers(vk);
    vulkan_make_command_buffers(vk);
}

internal void
vulkan_draw_frame(vulkan_context *vk)
{
    // Wait for fences
    VkFence fence = ((VkFence *)vk->swapChain.inFlightFences.data)[vk->swapChain.currentFrame];
    vkWaitForFences(vk->device, 1, &fence, VK_TRUE, UINT64_MAX);
    
    // Get semaphores
    u32 imageIndex;
    VkSemaphore imageSemaphore = ((VkSemaphore *)vk->swapChain.imageAvailableSemaphores.data)[vk->swapChain.currentFrame];
    VkSemaphore renderFinishedSemaphore = ((VkSemaphore *)vk->swapChain.renderFinishedSemaphores.data)[vk->swapChain.currentFrame];
    
    // Get image
    VkResult result = vkAcquireNextImageKHR(vk->device, vk->swapChain.object, UINT64_MAX, imageSemaphore, VK_NULL_HANDLE, &imageIndex);
    
    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (((VkFence *)vk->swapChain.imagesInFlight.data)[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(vk->device, 1, &((VkFence *)vk->swapChain.imagesInFlight.data)[imageIndex], VK_TRUE, UINT64_MAX);
    }
    
    // Mark the image as now being in use by this frame
    ((VkFence *)vk->swapChain.imagesInFlight.data)[imageIndex] = ((VkFence *)vk->swapChain.inFlightFences.data)[vk->swapChain.currentFrame];
    
    // Reset fences
    vkResetFences(vk->device, 1, &fence);
    
    // Submit queue
    VkSemaphore waitSemaphores[] = {imageSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    VkCommandBuffer *commandBuffers = &((VkCommandBuffer *)vk->commandBuffers.data)[imageIndex];
    VkSubmitInfo submitInfo = vulkan_get_submitinfo(waitSemaphores, waitStages, signalSemaphores, commandBuffers);
    if (vkQueueSubmit(vk->queue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        fatal_error("Failed to submit draw command buffer!");
    }
    
    // Present queue
    VkSwapchainKHR swapChains[] = {vk->swapChain.object};
    VkPresentInfoKHR presentInfo = vulkan_get_presentinfo_khr(signalSemaphores, swapChains, &imageIndex);
    vkQueuePresentKHR(vk->queue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || vk->swapChain.frameBufferResized)
    {
        vk->swapChain.frameBufferResized = 0;
        vulkan_remake_swapchain(vk);
    }
    
    vk->swapChain.currentFrame = (vk->swapChain.currentFrame + 1) % VULKAN_MAX_FRAMES_IN_FLIGHT;
}

#endif //WIN32_VULKAN_PRESENTATION_H
