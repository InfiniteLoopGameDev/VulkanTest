#pragma once

#include <optional>

#include <vulkan/vulkan_raii.hpp>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    QueueFamilyIndices(vk::raii::PhysicalDevice &physical_device, vk::raii::SurfaceKHR &surface) {
        std::vector<vk::QueueFamilyProperties> properties =
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

    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};
