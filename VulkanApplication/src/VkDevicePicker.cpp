#include "VkDevicePicker.h"

namespace nex {

VkPhysicalDevice VkDevicePicker::PickDevice(const VkDevices devices) {
    uint32_t maxDeviceSuitability = 0;
    VkPhysicalDevice suitableDevice = VK_NULL_HANDLE;

    for (const auto& device : devices.devices()) {
        uint32_t deviceSuitability = RateDeviceSuitability(device);

        if (deviceSuitability > maxDeviceSuitability) {
            maxDeviceSuitability = deviceSuitability;
            suitableDevice = device;
        }
    }

    return suitableDevice;
}

uint32_t VkDevicePicker::RateDeviceSuitability(VkPhysicalDevice device) {
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

QueueFamilyIndices VkDevicePicker::FindQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices queueFamilyIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (size_t queueFamilyIdx = 0; queueFamilyIdx < queueFamilies.size(); queueFamilyIdx++) {
        const auto queueFamily = queueFamilies[queueFamilyIdx];

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyIndices.graphicsFamily = queueFamilyIdx;
        }

        if (queueFamilyIndices.isComplete()) {
            break;
        }
    }

    return queueFamilyIndices;
}

} // namespace nex