#pragma once

#include <vulkan/vulkan_raii.hpp>

bool check_device_extensions(const vk::raii::PhysicalDevice &device,
                             std::vector<const char *> &requested_extensions);

VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_callback(
    [[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
    [[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT message_type,
    const vk::DebugUtilsMessengerCallbackDataEXT *callback_data, [[maybe_unused]] void *user_data);