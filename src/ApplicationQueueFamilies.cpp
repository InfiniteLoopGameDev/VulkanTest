#include "ApplicationQueueFamilies.h"

#include <vulkan/vulkan_raii.hpp>

ApplicationQueueFamilies::ApplicationQueueFamilies() = default;

ApplicationQueueFamilies::ApplicationQueueFamilies(
    const vk::raii::PhysicalDevice &physical_device, const vk::raii::SurfaceKHR &surface) {
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

bool ApplicationQueueFamilies::isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
}

bool ApplicationQueueFamilies::areUnique() const {
    assert(isComplete());
    return graphicsFamily.value() != presentFamily.value();
}

std::vector<uint32_t> ApplicationQueueFamilies::getQueueFamilyIndices() const {
    assert(isComplete());
    return {graphicsFamily.value(), presentFamily.value()};
}