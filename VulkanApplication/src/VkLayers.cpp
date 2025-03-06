#include "VkLayers.h"

namespace nex {


VkLayers VkLayers::InstanceLayers() {
    VkLayers layers;

    uint32_t layersCount;
    vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

    layers.m_layers.resize(layersCount);
    vkEnumerateInstanceLayerProperties(&layersCount, layers.m_layers.data());

    return layers;
}

bool VkLayers::layerAvailable(std::string_view layerName) {
    for (auto layer : m_layers) {
        if (layerName == layer.layerName) {
            return true;
        }
    }
    return false;
}

} // namespace nex