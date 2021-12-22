/* date = December 20th 2021 3:59 am */

#ifndef WIN32_VULKAN_CORE_H
#define WIN32_VULKAN_CORE_H

#define VULKAN_MAX_FRAMES_IN_FLIGHT 2

#include "win32_vulkan_types.h"

#ifdef VULKAN_USE_VALIDATION_LAYER
#include "win32_vulkan_vlayers.h"
#endif

internal VkApplicationInfo
vulkan_get_applicationinfo(void)
{
    VkApplicationInfo appInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = 0,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };
    
    return appInfo;
}

#include "win32_vulkan_queue.h"
#include "win32_vulkan_swapchain.h"
#include "win32_vulkan_device.h"
#include "win32_vulkan_instance.h"
#include "win32_vulkan_surface.h"
#include "win32_vulkan_shader.h"
#include "win32_vulkan_input.h"
#include "win32_vulkan_viewport.h"
#include "win32_vulkan_rasterizer.h"
#include "win32_vulkan_renderpass.h"
#include "win32_vulkan_pipeline.h"
#include "win32_vulkan_commands.h"
#include "win32_vulkan_presentation.h"

internal b32
vulkan_load_functions(vulkan_context *vk)
{
#ifdef VULKAN_USE_VALIDATION_LAYER
    vk->vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(vk->instance, "vkCreateDebugUtilsMessengerEXT");
    
    vk->vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(vk->instance, "vkDestroyDebugUtilsMessengerEXT");
    
    if (!vk->vkCreateDebugUtilsMessengerEXT ||
        !vk->vkDestroyDebugUtilsMessengerEXT)
    {
        return 0;
    }
#endif
    
    return 1;
}


internal void
vulkan_init(vulkan_context *vk)
{
    OutputDebugStringA("\n\n----------- INITIALIZING VULKAN ------------\n\n");
    
    // Must call before making instance
    vulkan_pre_instance_config(vk);
    
    // Make vulkan instance
    vulkan_make_instance(vk);
    
    // Load additional vulkan functions
    if (!vulkan_load_functions(vk))
    {
        fatal_error("Failed to load one or more vulkan functions. (I know, very useful.)");
    }
    
    // Setup debug system
#ifdef VULKAN_USE_VALIDATION_LAYER
    vulkan_setup_debug_messenger(vk);
#endif
    
    // Make surface before picking physical device
    vulkan_make_surface(vk);
    
    // Pick physical device and make logical device
    vulkan_pick_physical_device(vk);
    vulkan_make_logical_device(vk);
    vulkan_get_device_queue(vk);
    
    // Make swap chain and graphics pipeline
    vulkan_make_swapchain(vk);
    vulkan_make_graphics_pipeline(vk);
    vulkan_make_swapchain_framebuffers(vk);
    
    // Make command pool/buffers
    vulkan_make_command_pool(vk);
    vulkan_make_command_buffers(vk);
    
    // Make swapchain semaphores
    vulkan_make_swapchain_sync_objects(vk);
}

internal void
vulkan_cleanup(vulkan_context *vk)
{
    vkDeviceWaitIdle(vk->device);
    
#ifdef VULKAN_USE_VALIDATION_LAYER
    vk->vkDestroyDebugUtilsMessengerEXT(vk->instance, vk->debugMessenger, 0);
#endif
    
    vulkan_cleanup_swapchain(vk);
    
    for (u32 i = 0;
         i < VULKAN_MAX_FRAMES_IN_FLIGHT;
         ++i)
    {
        vkDestroySemaphore(vk->device, ((VkSemaphore *)vk->swapChain.renderFinishedSemaphores.data)[i], 0);
        vkDestroySemaphore(vk->device, ((VkSemaphore *)vk->swapChain.imageAvailableSemaphores.data)[i], 0);
        vkDestroyFence(vk->device, ((VkFence *)vk->swapChain.inFlightFences.data)[i], 0);
    }
    
    vkDestroyCommandPool(vk->device, vk->commandPool, 0);
    
    
    vkDestroyDevice(vk->device, 0);
    vkDestroySurfaceKHR(vk->instance, vk->surface, 0);
    vkDestroyInstance(vk->instance, 0);
}

#endif //WIN32_VULKAN_CORE_H
