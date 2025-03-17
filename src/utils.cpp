#include "utils.h"
#include <set>

bool check_device_extensions(const vk::raii::PhysicalDevice &device,
                             std::vector<const char *> &requested_extensions) {
    std::vector<vk::ExtensionProperties> available_extensions =
        device.enumerateDeviceExtensionProperties();
    std::set<std::string> required_extensions(requested_extensions.begin(),
                                              requested_extensions.end());

    for (auto &extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}