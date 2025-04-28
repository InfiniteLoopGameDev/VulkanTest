#include "ApplicationSwapChainDetails.h"

ApplicationSwapChainDetails::ApplicationSwapChainDetails() = default;

ApplicationSwapChainDetails::ApplicationSwapChainDetails(
    const vk::raii::PhysicalDevice &physical_device, const vk::raii::SurfaceKHR &surface) {
    capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);

    formats = physical_device.getSurfaceFormatsKHR(surface);
    presentModes = physical_device.getSurfacePresentModesKHR(surface);
}

bool ApplicationSwapChainDetails::isValid() const {
    return !formats.empty() && !presentModes.empty();
}