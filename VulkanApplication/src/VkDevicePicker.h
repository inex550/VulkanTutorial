#ifndef __VulkanApp_VkDevicePicker_H__
#define __VulkanApp_VkDevicePicker_H__

#include <vulkan/vulkan.h>

#include "VkDevices.h"

#include <optional>

namespace nex {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    
    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

class VkDevicePicker {
public:

public:
    VkDevicePicker() = delete;

    static VkPhysicalDevice PickDevice(const VkDevices devices);

    static uint32_t RateDeviceSuitability(VkPhysicalDevice device);

    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
};

} // namespace nex

#endif // __VulkanApp_VkDevicePicker_H__