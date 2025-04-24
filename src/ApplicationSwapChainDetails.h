#pragma once

#include <vulkan/vulkan_raii.hpp>

struct ApplicationSwapChainDetails {
    vk::SurfaceCapabilitiesKHR capabilities;

    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;

    ApplicationSwapChainDetails();

    ApplicationSwapChainDetails(const vk::raii::PhysicalDevice &physical_device,
                                const vk::raii::SurfaceKHR &surface);

    [[nodiscard]] bool isValid() const;
};