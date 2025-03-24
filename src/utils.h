#pragma once

#include <vulkan/vulkan_raii.hpp>

bool check_device_extensions(const vk::raii::PhysicalDevice &device,
                             std::vector<const char *> &requested_extensions);

VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
    [[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, [[maybe_unused]] void *pUserData);