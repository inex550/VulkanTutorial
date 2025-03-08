#include "VkDevices.h"

namespace nex {

VkDevices::VkDevices(VkInstance vkInstance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
    
    m_devices.resize(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, m_devices.data());
}

} // namespace nex