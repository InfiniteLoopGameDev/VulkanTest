#pragma once

#include <vulkan/vulkan_raii.hpp>

struct SwapChainDetails {
    vk::SurfaceCapabilitiesKHR capabilities;

    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;

    SwapChainDetails(const vk::raii::PhysicalDevice &physical_device,
                     const vk::raii::SurfaceKHR &surface) {
        capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);

        formats = physical_device.getSurfaceFormatsKHR(surface);
        presentModes = physical_device.getSurfacePresentModesKHR(surface);
    }

    [[nodiscard]] bool isValid() const { return !formats.empty() && !presentModes.empty(); }
};