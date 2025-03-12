#ifndef __VulkanApp_VkDevices_H__
#define __VulkanApp_VkDevices_H__

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

namespace nex {

struct DeviceQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct DeviceSwapChainInfo {
    VkSurfaceCapabilitiesKHR capabilities {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VkDeviceUtils {
public:
    VkDeviceUtils() = delete;

    static std::vector<VkPhysicalDevice> PhysicalDevices(VkInstance vkInstance);

    static DeviceQueueFamilyIndices FindDeviceQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    static DeviceSwapChainInfo GetDeviceSwapChainInfo(VkPhysicalDevice device, VkSurfaceKHR surface);

    static uint32_t RateDeviceSuitability(VkPhysicalDevice device);
};

} // namespace nex

#endif // __VulkanApp_VkDevices_H__