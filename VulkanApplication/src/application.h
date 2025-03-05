#ifndef __VulkanApp_Application_H__
#define __VulkanApp_Application_H__

#include <string_view>

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

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

    void cleanup();

    void loop();

    bool checkExtensionsAvailable(uint32_t extensionCount, const char** extensions);

private:
    bool m_init = false;

    std::string_view m_title;
    int m_width = 0;
    int m_height = 0;

    GLFWwindow* m_window = nullptr;

    VkInstance m_vkInstance;
};

} // namespace nex

#endif // __VulkanApp_Application_H__