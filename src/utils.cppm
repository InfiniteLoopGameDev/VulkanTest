module;

#include <iostream>
#include <set>

#include <sfml/System.hpp>
#include <vulkan/vk_platform.h>

export module utils;

import vulkan_hpp;

export std::vector<const char *> to_c_strings(const std::vector<std::string_view> &strings) {
    std::vector<const char *> c_strings;
    c_strings.reserve(strings.size());
    for (auto &string: strings) {
        c_strings.push_back(string.data());
    }
    return c_strings;
}

export bool check_device_extensions(const vk::raii::PhysicalDevice &device,
                                    std::vector<std::string_view> &requested_extensions) {
    const std::vector<vk::ExtensionProperties> available_extensions =
            device.enumerateDeviceExtensionProperties();
    std::set<std::string> required_extensions(requested_extensions.begin(),
                                              requested_extensions.end());

    for (auto &extension: available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
        [[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
        [[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT message_type,
        const vk::DebugUtilsMessengerCallbackDataEXT *callback_data, [[maybe_unused]] void *user_data) {

    std::cerr << "Validation layer: " << callback_data->pMessage << std::endl;

    return vk::False;
}

int rate_surface_format(const vk::SurfaceFormatKHR &surface_format) {
    // TODO: Try to actually get HDR formats
    int total = 0;

    if (surface_format.format == vk::Format::eR16G16B16A16Sfloat) {
        total += 1000;
    } else if (surface_format.format == vk::Format::eB8G8R8A8Srgb) {
        total += 500;
    } else {
        return 0;
    }

    if (surface_format.colorSpace == vk::ColorSpaceKHR::eDisplayP3NonlinearEXT) {
        total += 1000;
    } else if (surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
        total += 500;
    } else {
        return 0;
    }

    return total;
}

export vk::SurfaceFormatKHR
choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR> &available_formats) {
    int max = 0;
    vk::SurfaceFormatKHR best_format;

    for (auto &available_format: available_formats) {
        if (const int rating = rate_surface_format(available_format); rating > max) {
            best_format = available_format;
            max = rating;
        }
    }

    return best_format;
}

export vk::PresentModeKHR
choose_present_mode(const std::vector<vk::PresentModeKHR> &available_present_modes) {
    for (auto &available_present_mode: available_present_modes) {
        if (available_present_mode == vk::PresentModeKHR::eMailbox) {
            return available_present_mode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

export vk::Extent2D choose_swap_extent(const vk::SurfaceCapabilitiesKHR &capabilities,
                                       const sf::Vector2u &window_size) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    const vk::Extent2D extent(std::clamp(window_size.x, capabilities.minImageExtent.width,
                                         capabilities.maxImageExtent.width),
                              std::clamp(window_size.y, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height));
    return extent;
}

export std::vector<vk::raii::ImageView> create_image_views(const vk::raii::Device &device,
                                                           const std::vector<vk::Image> &swap_chain_images,
                                                           const vk::Format &swap_chain_image_format) {
    std::vector<vk::raii::ImageView> result;

    constexpr vk::ImageSubresourceRange subresource_range(vk::ImageAspectFlagBits::eColor,
                                                          0, 1, 0,
                                                          1);

    for (auto &image: swap_chain_images) {
        vk::ImageViewCreateInfo create_info({}, image, vk::ImageViewType::e2D,
                                            swap_chain_image_format, {}, subresource_range);

        result.emplace_back(device, create_info);
    }

    return result;
}

export constexpr vk::DebugUtilsMessengerCreateInfoEXT
        debugUtilsMessengerCreateInfo({},
                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                                      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                                      debug_callback);
