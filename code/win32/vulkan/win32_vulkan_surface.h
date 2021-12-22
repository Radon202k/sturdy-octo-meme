/* date = December 20th 2021 4:05 am */

#ifndef WIN32_VULKAN_SURFACE_H
#define WIN32_VULKAN_SURFACE_H

internal VkWin32SurfaceCreateInfoKHR
vulkan_get_surface_createinfo_khr(vulkan_context *vk)
{
    VkWin32SurfaceCreateInfoKHR createInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hwnd = vk->hwnd,
        .hinstance = GetModuleHandle(0),
    };
    
    return createInfo;
}

internal void
vulkan_make_surface(vulkan_context *vk)
{
    VkWin32SurfaceCreateInfoKHR createInfo = 
        vulkan_get_surface_createinfo_khr(vk);
    
    if (vkCreateWin32SurfaceKHR(vk->instance, &createInfo, 0, &vk->surface) != VK_SUCCESS)
    {
        fatal_error("Failed to create window surface!");
    }
}

#endif //WIN32_VULKAN_SURFACE_H
