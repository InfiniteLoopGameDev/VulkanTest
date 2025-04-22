#include "SwapChainDetails.h"

SwapChainDetails::SwapChainDetails() = default;

SwapChainDetails::SwapChainDetails(const vk::raii::PhysicalDevice &physical_device,
                                   const vk::raii::SurfaceKHR &surface) {
    capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);

    formats = physical_device.getSurfaceFormatsKHR(surface);
    presentModes = physical_device.getSurfacePresentModesKHR(surface);
}

bool SwapChainDetails::isValid() const { return !formats.empty() && !presentModes.empty(); }