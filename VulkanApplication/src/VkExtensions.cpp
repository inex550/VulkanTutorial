#include "VkExtensions.h"

namespace nex {

VkExtensions VkExtensions::InstanceExtensions() {
    VkExtensions extensions;

    uint32_t extensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

    extensions.m_extensions.resize(extensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.m_extensions.data());

    return extensions;
}

bool VkExtensions::extensionAvailable(const std::string_view& extensionName) {
    for (auto extension : m_extensions) {
        if (extensionName == extension.extensionName) {
            return true;
        }
    }
    return false;
}

} // namespace nex