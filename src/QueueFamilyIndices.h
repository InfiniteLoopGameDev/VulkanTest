#pragma once

#include <optional>

#include <vulkan/vulkan_raii.hpp>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    QueueFamilyIndices();

    QueueFamilyIndices(const vk::raii::PhysicalDevice &physical_device,
                       const vk::raii::SurfaceKHR &surface);

    [[nodiscard]] bool isComplete() const;

    [[nodiscard]] bool areUnique() const;

    [[nodiscard]] std::vector<uint32_t> getQueueFamilies() const;
};
