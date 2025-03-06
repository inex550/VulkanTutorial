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

    // GLFW
    GLFWwindow* m_window = nullptr;

    // Vulkan
    VkInstance m_vkInstance;
    VkDebugUtilsMessengerEXT m_vkDebugMessengerInstance;

    VkExtensions m_instanceExtensions = VkExtensions::InstanceExtensions();
    VkLayers m_instanceLayers = VkLayers::InstanceLayers();
};

} // namespace nex

#endif // __VulkanApp_Application_H__