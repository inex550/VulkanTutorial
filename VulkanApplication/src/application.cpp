#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>

#include "application.h"

#include <iostream>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <vector>
#include <array>
#include <set>

#include "VkDevices.h"
#include "Utils.h"

#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

#define SHADER_VERT_CODE_FILE "assets/triangle.vert.spv"
#define SHADER_FRAG_CODE_FILE "assets/triangle.frag.spv"

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
    createGraphicsPipeline();
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
    createInfo.messageSeverity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
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

    uint32_t maxDeviceSuitability = 0;
    VkPhysicalDevice bestSuitableDevice = VK_NULL_HANDLE;

    for (const auto& device : physicalDevices) {
        VkExtensions deviceExtensions = VkExtensions::DeviceExtensions(device);
        if (!deviceExtensions.extensionsAvailable(m_requiredDeviceExtensions.begin(), m_requiredDeviceExtensions.end())) {
            continue;
        }

        DeviceSwapChainInfo deviceSwapChainInfo = VkDeviceUtils::GetDeviceSwapChainInfo(device, m_vkSurface);
        if (deviceSwapChainInfo.formats.empty() || deviceSwapChainInfo.presentModes.empty()) {
            continue;
        }

        uint32_t deviceSuitability = VkDeviceUtils::RateDeviceSuitability(device);
        
        if (deviceSuitability > maxDeviceSuitability) {
            maxDeviceSuitability = deviceSuitability;
            bestSuitableDevice = device;
        }
    }

    m_pickedVkPhysicalDevice = bestSuitableDevice;

    if (m_pickedVkPhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find any suitable physical device");
    }
}

void Application::createVulkanLogicalDevice() {
    DeviceQueueFamilyIndices deviceQueueFamilyIndices = VkDeviceUtils::FindDeviceQueueFamilies(m_pickedVkPhysicalDevice, m_vkSurface);

    if (!deviceQueueFamilyIndices.isComplete()) {
        throw std::runtime_error("Failed to find necessary queue family");
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    std::set<uint32_t> uniqueQueueFamilyIndices { 
        deviceQueueFamilyIndices.graphicsFamily.value(), deviceQueueFamilyIndices.presentFamily.value() };

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

    deviceCreateInfo.ppEnabledExtensionNames = m_requiredDeviceExtensions.data();
    deviceCreateInfo.enabledExtensionCount = m_requiredDeviceExtensions.size();

    deviceCreateInfo.ppEnabledLayerNames = m_requiredInstanceLayers.data();
    deviceCreateInfo.enabledLayerCount = m_requiredInstanceLayers.size();

    if (VkResult result = vkCreateDevice(m_pickedVkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice); result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(m_vkDevice, deviceQueueFamilyIndices.graphicsFamily.value(), 0, &m_vkGraphicsQueue);
    vkGetDeviceQueue(m_vkDevice, deviceQueueFamilyIndices.presentFamily.value(), 0, &m_vkPresentQueue);
}

void Application::createSwapChain() {
    DeviceSwapChainInfo swapChainInfo = VkDeviceUtils::GetDeviceSwapChainInfo(m_pickedVkPhysicalDevice, m_vkSurface);

    VkSurfaceFormatKHR choosedSurfaceFormat = chooseSurfaceFormat(swapChainInfo.formats);
    VkPresentModeKHR choosedPresentMode = choosePresentMode(swapChainInfo.presentModes);
    VkExtent2D choosedSwapchainExtent = chooseSwapExtent(swapChainInfo.capabilities);

    uint32_t imageCount = swapChainInfo.capabilities.minImageCount + 1;
    if (swapChainInfo.capabilities.maxImageCount > 0 && imageCount <= swapChainInfo.capabilities.maxImageCount) {
        imageCount = swapChainInfo.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageFormat = choosedSurfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = choosedSurfaceFormat.colorSpace;
    swapChainCreateInfo.presentMode = choosedPresentMode;
    swapChainCreateInfo.imageExtent = choosedSwapchainExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    DeviceQueueFamilyIndices queueFamilyIndices = VkDeviceUtils::FindDeviceQueueFamilies(m_pickedVkPhysicalDevice, m_vkSurface);

    std::array<uint32_t, 2> indices { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };

    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = indices.size();
        swapChainCreateInfo.pQueueFamilyIndices = indices.data();
    } else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapChainCreateInfo.preTransform = swapChainInfo.capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.clipped = VK_TRUE;
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (VkResult result = vkCreateSwapchainKHR(m_vkDevice, &swapChainCreateInfo, nullptr, &m_vkSwapchain); result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vulkan swapchain");
    }

    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &swapchainImageCount, nullptr);
    m_swapchainImages.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &swapchainImageCount, m_swapchainImages.data());

    m_swapchainImageFormat = choosedSurfaceFormat;
    m_swapchainImageExtent = choosedSwapchainExtent;
}

void Application::createImageViews() {
    m_swapchainImageViews.resize(m_swapchainImages.size());

    for (size_t i = 0; i < m_swapchainImageViews.size(); ++i) {
        VkImageViewCreateInfo imageViewCreateInfo {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = m_swapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

        imageViewCreateInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;

        imageViewCreateInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if (VkResult result = vkCreateImageView(m_vkDevice, &imageViewCreateInfo, nullptr, &m_swapchainImageViews[i]); result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create vulkan image view");
        }
    }
}

void Application::createGraphicsPipeline() {
    auto shaderVertCode = utils::ReadFile(SHADER_VERT_CODE_FILE);
    auto shaderFragCode = utils::ReadFile(SHADER_FRAG_CODE_FILE);

    auto shaderVertModule = createShaderModule(shaderVertCode);
    auto shaderFragModule = createShaderModule(shaderFragCode);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.pName = "main";
    vertShaderStageCreateInfo.module = shaderVertModule;

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
    vertShaderStageCreateInfo.pName = "main";
    vertShaderStageCreateInfo.module = shaderFragModule;

    VkPipelineShaderStageCreateInfo shaderStageCreateInfos[] = { vertShaderStageCreateInfo, fragShaderStageCreateInfo };

    std::array<VkDynamicState, 2> dynamicStates = {
        VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
        VkDynamicState::VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo pipelineStateCreateInfo {};
    pipelineStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineStateCreateInfo.dynamicStateCount = dynamicStates.size();
    pipelineStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyCreateInfo;
    pipelineInputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssemblyCreateInfo.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchainImageExtent.width);
    viewport.height = static_cast<float>(m_swapchainImageExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.offset = { 0, 0 };
    scissor.extent = m_swapchainImageExtent;

    VkPipelineViewportStateCreateInfo pipelineViewportCreateInfo {};
    pipelineViewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineViewportCreateInfo.viewportCount = 1;
    pipelineViewportCreateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo {};
    pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    pipelineRasterizationStateCreateInfo.cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
    pipelineRasterizationStateCreateInfo.frontFace = VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
    pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo pipelineMultisableStateCreateInfo {};
    pipelineMultisableStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineMultisableStateCreateInfo.sampleShadingEnable = VK_FALSE;
    pipelineMultisableStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineMultisableStateCreateInfo.minSampleShading = 1.0f;
    pipelineMultisableStateCreateInfo.pSampleMask = nullptr;
    pipelineMultisableStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    pipelineMultisableStateCreateInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachment {};
    pipelineColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipelineColorBlendAttachment.blendEnable = VK_FALSE;
    pipelineColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    pipelineColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipelineColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    pipelineColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipelineColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipelineColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo {};
    pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    pipelineColorBlendStateCreateInfo.attachmentCount = 1;
    pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelieLayoutCreateInfo {};
    pipelieLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (VkResult result = vkCreatePipelineLayout(m_vkDevice, &pipelieLayoutCreateInfo, nullptr, &m_vkPipelineLayout); result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create VkPipelineLayout");
    }
}

VkShaderModule Application::createShaderModule(const std::vector<char>& shaderCode) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shaderCode.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (VkResult result = vkCreateShaderModule(m_vkDevice, &shaderModuleCreateInfo, nullptr, &shaderModule); result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create VkShaderModule");
    }

    return shaderModule;
}

VkSurfaceFormatKHR Application::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (auto surfaceFormat : availableFormats) {
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return surfaceFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR Application::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (auto presentMode : availablePresentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetFramebufferSize(m_window, &windowWidth, &windowHeight);

    VkExtent2D extent {
        .width = static_cast<uint32_t>(windowWidth),
        .height = static_cast<uint32_t>(windowHeight)
    };

    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
}

void Application::cleanup() {
    if (!m_init) {
        return;
    }

    for (auto& imageView : m_swapchainImageViews) {
        vkDestroyImageView(m_vkDevice, imageView, nullptr);
    }
    m_swapchainImageViews.clear();

    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapchain, nullptr);
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