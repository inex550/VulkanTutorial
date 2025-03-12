#include "VkDevices.h"

namespace nex {

std::vector<VkPhysicalDevice> VkDeviceUtils::PhysicalDevices(VkInstance vkInstance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    return devices;
}

DeviceQueueFamilyIndices VkDeviceUtils::FindDeviceQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    DeviceQueueFamilyIndices queueFamilyIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (size_t queueFamilyIdx = 0; queueFamilyIdx < queueFamilies.size(); queueFamilyIdx++) {
        const auto queueFamily = queueFamilies[queueFamilyIdx];
        
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyIndices.graphicsFamily = queueFamilyIdx;
        }

        VkBool32 surfaceSupported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIdx, surface, &surfaceSupported);

        if (surfaceSupported) {
            queueFamilyIndices.presentFamily = queueFamilyIdx;
        }

        if (queueFamilyIndices.isComplete()) {
            break;
        }
    }

    return queueFamilyIndices;
}

DeviceSwapChainInfo VkDeviceUtils::GetDeviceSwapChainInfo(VkPhysicalDevice device, VkSurfaceKHR surface) {
    DeviceSwapChainInfo swapChainInfo;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainInfo.capabilities);

    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, nullptr);
    
    swapChainInfo.formats.resize(formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, swapChainInfo.formats.data());

    uint32_t presentModesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);
    
    swapChainInfo.presentModes.resize(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, swapChainInfo.presentModes.data());

    return swapChainInfo;
}

uint32_t VkDeviceUtils::RateDeviceSuitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    uint32_t deviceScore = 0;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        deviceScore += 10;
    }

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        deviceScore += 20;
    }

    return deviceScore;
}

} // namespace nex