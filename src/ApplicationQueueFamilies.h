#pragma once

#include <vector>

namespace vk::raii {
class PhysicalDevice;
class SurfaceKHR;
} // namespace vk::raii

struct ApplicationQueueFamilies {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    ApplicationQueueFamilies();

    ApplicationQueueFamilies(const vk::raii::PhysicalDevice &physical_device,
                             const vk::raii::SurfaceKHR &surface);

    [[nodiscard]] bool isComplete() const;

    [[nodiscard]] bool areUnique() const;

    [[nodiscard]] std::vector<uint32_t> getQueueFamilyIndices() const;
};
