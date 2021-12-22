/* date = December 20th 2021 4:13 am */

#ifndef WIN32_VULKAN_TYPES_H
#define WIN32_VULKAN_TYPES_H

typedef struct vulkan_swapchain_support_details
{
    VkSurfaceCapabilitiesKHR capabilities;
    typeless_vector formats;
    typeless_vector presentModes;
} vulkan_swapchain_support_details;

#ifdef VULKAN_USE_VALIDATION_LAYER
typedef struct vulkan_vlayers
{
    u32 count;
    VkLayerProperties *props;
} vulkan_vlayers;
#endif

typedef struct vulkan_swapchain
{
    VkSwapchainKHR object;
    VkFormat imageFormat;
    VkExtent2D extent;
    typeless_vector images;
    typeless_vector imageViews;
    typeless_vector frameBuffers;
    typeless_vector imageAvailableSemaphores;
    typeless_vector renderFinishedSemaphores;
    typeless_vector inFlightFences;
    typeless_vector imagesInFlight;
    u32 currentFrame;
    b32 frameBufferResized;
} vulkan_swapchain;

typedef struct vulkan_context
{
    HWND hwnd;
    u32 windowWidth;
    u32 windowHeight;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;
    typeless_vector deviceQueueFamilies;
    u32 deviceQueueFamilyIndex;
    VkSurfaceKHR surface;
    vulkan_swapchain swapChain;
    VkPipelineLayout graphicsPipelineLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    typeless_vector commandBuffers;
    
    // Arrays of required extensions defined by user
    string_array requiredInstanceExtensions;
    string_array requiredDeviceExtensions;
    
#ifdef VULKAN_USE_VALIDATION_LAYER
    vulkan_vlayers vLayers;
    string_array vLayersNamesArray;
    
    VkDebugUtilsMessengerEXT debugMessenger;
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
#endif
    
} vulkan_context;

#endif //WIN32_VULKAN_TYPES_H
