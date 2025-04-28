module;

#include <cassert>
#include <cstdint>

export module vkt.Application:QueueFamilies;

import std;

import vulkan_hpp;

export struct ApplicationQueueFamilies {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    ApplicationQueueFamilies() = default;

    ApplicationQueueFamilies(const vk::raii::PhysicalDevice &physical_device,
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

    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    [[nodiscard]] bool areUnique() const {
        assert(isComplete());
        return graphicsFamily.value() != presentFamily.value();
    }

    [[nodiscard]] std::vector<uint32_t> getQueueFamilyIndices() const {
        assert(isComplete());
        return {graphicsFamily.value(), presentFamily.value()};
    }
};