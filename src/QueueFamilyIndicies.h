#pragma once

#include <optional>

#include <vulkan/vulkan_raii.hpp>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    QueueFamilyIndices(const vk::raii::PhysicalDevice &physical_device,
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
};
