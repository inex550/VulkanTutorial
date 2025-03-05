#include "application.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>

namespace nex {

Application::Application(std::string_view title, int width, int height)
    : m_title(title)
    , m_width(width)
    , m_height(height)
{
}

Application::~Application() {
    if (m_init) {
        cleanup();
    }
}

void Application::run() {
    init();
    loop();
}

void Application::init() {
    if (m_init) {
        return;
    }

    initWindow();
    initVulkan();
}

void Application::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    glfwSetErrorCallback([](int code, const char* description) {
        std::cerr << "GLFW error (" << code << "): " << description << std::endl;
    });

    m_window = glfwCreateWindow(m_width, m_height, m_title.data(), nullptr, nullptr);
}

void Application::initVulkan() {
    createVulkanInstance();
}

void Application::createVulkanInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_title.data();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "NONE";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionsCount = 0;
    const char** glfwExtensions = nullptr;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    instanceCreateInfo.enabledExtensionCount = glfwExtensionsCount;
    instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

    if (!checkExtensionsAvailable(glfwExtensionsCount, glfwExtensions)) {
        throw std::runtime_error("Not all GLFW required extensions is available");
    }

    if (VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vkInstance); result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vulkan instance");
    }
}

void Application::cleanup() {
    if (!m_init) {
        return;
    }

    vkDestroyInstance(m_vkInstance, nullptr);

    glfwDestroyWindow(m_window);

    glfwTerminate();
}

void Application::loop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

bool Application::checkExtensionsAvailable(uint32_t extensionCount, const char** extensions) {
    uint32_t availableExtensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionsCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(availableExtensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionsCount, availableExtensions.data());

    for (int extensionIdx = 0; extensionIdx < extensionCount; extensionIdx++) {
        bool found = false;

        for (const auto& availableExtension : availableExtensions) {
            if (strcmp(availableExtension.extensionName, extensions[extensionIdx]) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

} // namespace nex