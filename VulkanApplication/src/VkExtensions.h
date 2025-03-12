#ifndef __VukanApp_VkExtensions_H__
#define __VukanApp_VkExtensions_H__

#include <vector>
#include <string_view>

#include <vulkan/vulkan.h>

namespace nex {

class VkExtensions {
public:
    VkExtensions() = default;

    static VkExtensions InstanceExtensions();
    static VkExtensions DeviceExtensions(VkPhysicalDevice device);

    bool extensionAvailable(const std::string_view& extensionName);

    template <typename Iter>
    bool extensionsAvailable(Iter begin, Iter end) {
        for (Iter iter = begin; iter < end; ++iter) {
            if (!extensionAvailable(*iter)) {
                return false;
            }
        }
        return true;
    }

public:
    const std::vector<VkExtensionProperties>& extensions() const {
        return m_extensions;
    }

private:
    std::vector<VkExtensionProperties> m_extensions;
};

} // namespace nex

#endif // __VukanApp_VkExtensions_H__