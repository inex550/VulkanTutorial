#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>

#include "application.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <set>

#include "VkDevices.h"

#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

namespace nex {

VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    std::string_view severity = "NONE";
    switch (messageSeverity) {
    #define CASE_SEVERITY(sev) case VK_DEBUG_UTILS_MESSAGE_SEVERITY_##sev##_BIT_EXT: severity = #sev; break
        CASE_SEVERITY(VERBOSE);
        CASE_SEVERITY(INFO);
        CASE_SEVERITY(WARNING);
        CASE_SEVERITY(ERROR);
    #undef CASE_SEVERITY
        default: break;
    }

    std::cerr << "[" << severity << "] -- ";

    #define CHECK_TYPE(type) \
    if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_##type##_BIT_EXT) {  \
        std::cerr << '[' << #type << ']';                               \
    }
    CHECK_TYPE(GENERAL);
    CHECK_TYPE(VALIDATION);
    CHECK_TYPE(PERFORMANCE);
    #undef CHECK_TYPE

    std::cerr << " -- " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

Application::Application(std::string_view title, int width, int height)
    : m_title(title)
    , m_width(width)
    , m_height(height)
{
}

Application::~Application() {
    cleanup();
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
    glfwSetErrorCallback([](int code, const char* description) {
        std::cerr << "GLFW error (" << code << "): " << description << std::endl;
    });

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    m_window = glfwCreateWindow(m_width, m_height, m_title.data(), nullptr, nullptr);

    // Get required for window vulkan instance extensions
    uint32_t glfwExtensionsCount = 0;
    const char** glfwExtensions = nullptr;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    m_requiredInstanceExtensions.insert(
        m_requiredInstanceExtensions.end(),
        glfwExtensions,
        glfwExtensions+glfwExtensionsCount
    );
}

void Application::initVulkan() {
    createVulkanInstance();
    createVulkanDebugMessenger();
    createVulkanSurface();
    pickVulkanPhysicalDevice();
    createVulkanLogicalDevice();
}

void Application::createVulkanInstance() {
    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_title.data();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "NONE";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    if (!m_instanceExtensions.extensionsAvailable(m_requiredInstanceExtensions.begin(), m_requiredInstanceExtensions.end())) {
        throw std::runtime_error("Not all required extensions is available");
    }

    instanceCreateInfo.enabledExtensionCount = m_requiredInstanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = m_requiredInstanceExtensions.data();

    if (!m_instanceLayers.layersAvailable(m_requiredInstanceLayers.begin(), m_requiredInstanceLayers.end())) {
        throw std::runtime_error("Not all required layers is available");
    }

    instanceCreateInfo.enabledLayerCount = m_requiredInstanceLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = m_requiredInstanceLayers.data();

    if (VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vkInstance); result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vulkan instance");
    }
}

void Application::createVulkanDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = vulkanDebugCallback;
    createInfo.pUserData = nullptr;

    auto createDebugMessengerFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugUtilsMessengerEXT")
    );
    if (!createDebugMessengerFunc) {
        std::cerr << "Function \"vkCreateDebugUtilsMessengerEXT\" can't be loaded" << std::endl;
    }

    if (VkResult result = createDebugMessengerFunc(m_vkInstance, &createInfo, nullptr, &m_vkDebugMessenger); result != VK_SUCCESS) {
        std::cerr << "createDebugUtilsMessenger func failed with code " << result << std::endl;
    }
}

void Application::createVulkanSurface() {
    VkWaylandSurfaceCreateInfoKHR createSurfaceInfo {};
    createSurfaceInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createSurfaceInfo.surface = glfwGetWaylandWindow(m_window);
    createSurfaceInfo.display = glfwGetWaylandDisplay();

    if (VkResult result = vkCreateWaylandSurfaceKHR(m_vkInstance, &createSurfaceInfo, nullptr, &m_vkSurface); result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vulkan wayland surface");
    }
}

void Application::pickVulkanPhysicalDevice() {
    std::vector<VkPhysicalDevice> physicalDevices = VkDeviceUtils::PhysicalDevices(m_vkInstance);
    m_pickedVkPhysicalDevice = VkDevicePicker::PickDevice(physicalDevices);

    if (m_pickedVkPhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find any suitable physical device");
    }
}

void Application::createVulkanLogicalDevice() {
    QueueFamilyIndices deviceQueueFamilyIndices = VkDeviceUtils::FindQueueFamilies(m_pickedVkPhysicalDevice, m_vkSurface);

    if (!deviceQueueFamilyIndices.isComplete()) {
        throw std::runtime_error("Failed to find necessary queue family");
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    std::set<uint32_t> uniqueQueueFamilyIndices { 
        deviceQueueFamilyIndices.graphicsFamily, deviceQueueFamilyIndices.presentFamily };

    for (uint32_t queueFamilyIdx : uniqueQueueFamilyIndices) {
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = deviceQueueFamilyIndices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        
        float queuePriority = 1.0;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};

    VkDeviceCreateInfo deviceCreateInfo {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    deviceCreateInfo.ppEnabledLayerNames = m_requiredInstanceLayers.data();
    deviceCreateInfo.enabledLayerCount = m_requiredInstanceLayers.size();

    if (VkResult result = vkCreateDevice(m_pickedVkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice); result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(m_vkDevice, deviceQueueFamilyIndices.graphicsFamily.value(), 0, &m_vkGraphicsQueue);
    vkGetDeviceQueue(m_vkDevice, deviceQueueFamilyIndices.presentFamily.value(), 0, &m_vkPresentQueue);
}

void Application::cleanup() {
    if (!m_init) {
        return;
    }

    vkDestroyDevice(m_vkDevice, nullptr);
    vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);    
    destroyVulkanDebugMessenger();
    vkDestroyInstance(m_vkInstance, nullptr);

    glfwDestroyWindow(m_window);

    glfwTerminate();
}

void Application::destroyVulkanDebugMessenger() {
    auto destroyDebugMessengerFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT")
    );
    if (destroyDebugMessengerFunc) {
        destroyDebugMessengerFunc(m_vkInstance, m_vkDebugMessenger, nullptr);
    }
}

void Application::loop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

} // namespace nex