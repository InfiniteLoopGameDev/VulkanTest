#pragma once

#include <vulkan/vulkan_raii.hpp>

bool check_device_extensions(const vk::raii::PhysicalDevice &device,
                             std::vector<const char *> &requested_extensions);