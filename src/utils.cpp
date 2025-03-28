#include "utils.h"

#include <iostream>
#include <set>

std::vector<const char *> to_c_strings(const std::vector<std::string_view> &strings) {
    std::vector<const char *> c_strings;
    c_strings.reserve(strings.size());
    for (auto &string : strings) {
        c_strings.push_back(string.data());
    }
    return c_strings;
}

bool check_device_extensions(const vk::raii::PhysicalDevice &device,
                             std::vector<std::string_view> &requested_extensions) {
    const std::vector<vk::ExtensionProperties> available_extensions =
        device.enumerateDeviceExtensionProperties();
    std::set<std::string> required_extensions(requested_extensions.begin(),
                                              requested_extensions.end());

    for (auto &extension : available_extensions) {
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