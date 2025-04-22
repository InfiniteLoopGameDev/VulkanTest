#include "QueueFamilyIndices.h"

QueueFamilyIndices::QueueFamilyIndices() = default;

QueueFamilyIndices::QueueFamilyIndices(const vk::raii::PhysicalDevice &physical_device,
                                       const vk::raii::SurfaceKHR &surface) {
    const std::vector<vk::QueueFamilyProperties> properties =
        physical_device.getQueueFamilyProperties();
    int i = 0;
    for (auto property : properties) {
        if (property.queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsFamily = i;
        }
        if (physical_device.getSurfaceSupportKHR(i, surface)) {
            presentFamily = i;
        }
        if (isComplete()) {
            break;
        }
        i++;
    }
}

bool QueueFamilyIndices::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
}

bool QueueFamilyIndices::areUnique() const {
    assert(isComplete());
    return graphicsFamily.value() != presentFamily.value();
}

std::vector<uint32_t> QueueFamilyIndices::getQueueFamilies() const {
    assert(isComplete());
    return {graphicsFamily.value(), presentFamily.value()};
}