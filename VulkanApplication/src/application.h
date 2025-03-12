#ifndef __VulkanApp_Application_H__
#define __VulkanApp_Application_H__

#include <string_view>

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include "VkExtensions.h"
#include "VkLayers.h"

#define ENABLE_VALIDATION_LAYERS

namespace nex {

class Application {
public:
    Application(std::string_view title, int width, int height);
    ~Application();

    void run();

private:
    void init();
    void initWindow();
    void initVulkan();

    void createVulkanInstance();
    void createVulkanDebugMessenger();
    void createVulkanSurface();
    void pickVulkanPhysicalDevice();
    void createVulkanLogicalDevice();
    void createSwapChain();
    void createImageViews();

    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void cleanup();
    void destroyVulkanDebugMessenger();

    void loop();

private:
    bool m_init = false;

    std::string_view m_title;
    int m_width = 0;
    int m_height = 0;

    std::vector<const char*> m_requiredInstanceExtensions {
        #ifdef ENABLE_VALIDATION_LAYERS
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        #endif
    };

    std::vector<const char*> m_requiredInstanceLayers {
        #ifdef ENABLE_VALIDATION_LAYERS
        "VK_LAYER_KHRONOS_validation"
        #endif
    };

    std::vector<const char*> m_requiredDeviceExtensions {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // GLFW
    GLFWwindow* m_window = nullptr;

    // Vulkan
    VkInstance m_vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_vkDebugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice m_pickedVkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_vkDevice = VK_NULL_HANDLE;
    VkSurfaceKHR m_vkSurface = VK_NULL_HANDLE;

    VkSwapchainKHR m_vkSwapchain = VK_NULL_HANDLE;
    VkSurfaceFormatKHR m_swapchainImageFormat {};
    VkExtent2D m_swapchainImageExtent {};
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    
    VkQueue m_vkGraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_vkPresentQueue = VK_NULL_HANDLE;

    VkExtensions m_instanceExtensions = VkExtensions::InstanceExtensions();
    VkLayers m_instanceLayers = VkLayers::InstanceLayers();
};

} // namespace nex

#endif // __VulkanApp_Application_H__