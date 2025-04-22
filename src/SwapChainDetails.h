#pragma once

#include <vulkan/vulkan_raii.hpp>

struct SwapChainDetails {
    vk::SurfaceCapabilitiesKHR capabilities;

    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;

    SwapChainDetails();

    SwapChainDetails(const vk::raii::PhysicalDevice &physical_device,
                     const vk::raii::SurfaceKHR &surface);

    [[nodiscard]] bool isValid() const;
};