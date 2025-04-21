#pragma once

#include <SFML/System.hpp>
#include <vulkan/vulkan_raii.hpp>

std::vector<const char *> to_c_strings(const std::vector<std::string_view> &strings);

bool check_device_extensions(const vk::raii::PhysicalDevice &device,
                             std::vector<std::string_view> &requested_extensions);

VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
    [[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
    [[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT message_type,
    const vk::DebugUtilsMessengerCallbackDataEXT *callback_data, [[maybe_unused]] void *user_data);

[[nodiscard]] vk::SurfaceFormatKHR
choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR> &available_formats);

[[nodiscard]] vk::PresentModeKHR
choose_present_mode(const std::vector<vk::PresentModeKHR> &available_present_modes);

[[nodiscard]] vk::Extent2D choose_swap_extent(const vk::SurfaceCapabilitiesKHR &capabilities,
                                              const sf::Vector2u &window_size);

[[nodiscard]] std::vector<vk::raii::ImageView>
create_image_views(const vk::raii::Device &device, const std::vector<vk::Image> &swap_chain_images,
                   const vk::Format &swap_chain_image_format);

constexpr vk::DebugUtilsMessengerCreateInfoEXT
    debugUtilsMessengerCreateInfo({},
                                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                                  vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
                                  debug_callback);