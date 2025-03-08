#ifndef __VulkanApp_VkDevices_H__
#define __VulkanApp_VkDevices_H__

#include <vulkan/vulkan.h>

#include <vector>

namespace nex {

class VkDevices {
public:
    VkDevices(VkInstance vkInstance);

public:
    const std::vector<VkPhysicalDevice>& devices() const {
        return m_devices;
    }

private:
    std::vector<VkPhysicalDevice> m_devices;
};

} // namespace nex

#endif // __VulkanApp_VkDevices_H__