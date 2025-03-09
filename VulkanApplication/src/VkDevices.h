#ifndef __VulkanApp_VkDevices_H__
#define __VulkanApp_VkDevices_H__

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

namespace nex {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class VkDeviceUtils {
public:
    VkDeviceUtils() = delete;

    static std::vector<VkPhysicalDevice> PhysicalDevices(VkInstance vkInstance);

    static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
};

class VkDevicePicker {
    public:
        VkDevicePicker() = delete;
    
        static VkPhysicalDevice PickDevice(const std::vector<VkPhysicalDevice>& devices);
    
        static uint32_t RateDeviceSuitability(VkPhysicalDevice device);
};

} // namespace nex

#endif // __VulkanApp_VkDevices_H__