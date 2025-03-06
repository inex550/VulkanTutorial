#ifndef __VulkanApp_VkLayers_H__
#define __VulkanApp_VkLayers_H__

#include <vector>
#include <string_view>

#include <vulkan/vulkan.h>

namespace nex {

class VkLayers {
public:
    VkLayers() = default;

    static VkLayers InstanceLayers();

    bool layerAvailable(std::string_view layerName);

    template <typename Iter>
    bool layersAvailable(Iter begin, Iter end) {
        for (Iter iter = begin; iter < end; ++iter) {
            if (!layerAvailable(*iter)) {
                return false;
            }
        }
        return true;
    }

public:
    const std::vector<VkLayerProperties>& layers() const {
        return m_layers;
    }

private:
    std::vector<VkLayerProperties> m_layers;
};

} // namespace nex

#endif // __VulkanApp_VkLayers_H__