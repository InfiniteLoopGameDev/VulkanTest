#include <iostream>
#include <set>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "QueueFamilyIndicies.h"

#include "Application.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "utils.h"

#ifndef NDEBUG
#define VALIDATION_LAYERS // CMake only sets NDEBUG on Release builds
#endif

Application::Application() {
    window.create(sf::VideoMode({1280, 720}), "Triangle", sf::Style::Close);
    initVulkan();
}

Application::~Application() = default;

void Application::run() { mainLoop(); }

void Application::initVulkan() {
    VULKAN_HPP_DEFAULT_DISPATCHER.init();

    const auto layers = selectLayers();
    std::vector<char const *> c_layers;
    c_layers.reserve(layers.size());
    for (auto &layer : layers) {
        c_layers.push_back(layer.c_str());
    }

    const auto layer_extensions = selectExtensions();
    std::vector<const char *> c_layer_extensions;
    c_layer_extensions.reserve(layer_extensions.size());
    for (auto &extension : layer_extensions) {
        c_layer_extensions.push_back(extension.c_str());
    }

    createInstance(c_layers, c_layer_extensions);

#ifdef VALIDATION_LAYERS
    setupDebugMessenger();
#endif

    createSurface();

    std::vector c_device_extensions = {vk::KHRSwapchainExtensionName};
    selectPhysicalDevice(c_device_extensions);

    createLogicalDevice(c_layers, c_device_extensions);
}

void Application::createInstance(const std::vector<const char *> &layers,
                                  const std::vector<const char *> &extensions) {
    const auto vk_version = vk::enumerateInstanceVersion();
    std::cout << "Vulkan version: " << vk::apiVersionMajor(vk_version) << "."
        << vk::apiVersionMinor(vk_version) << std::endl;

    constexpr vk::ApplicationInfo application_info("Triangle", vk::makeApiVersion(0, 0, 1, 0),
                                                   "No Engine", vk::makeApiVersion(0, 0, 1, 0),
                                                   vk::ApiVersion12);

    auto instance_flags = vk::InstanceCreateFlags();

#ifdef __APPLE__
    // Add VK_KHR_PORTABILITY_subset extension for MoltenVK
    instance_flags |= vk::raii::InstanceCreateFlagBits(
        vk::raii::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif

    const vk::InstanceCreateInfo
        instance_info(instance_flags, &application_info, layers, extensions);

    instance = vk::raii::Instance(context, instance_info);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
}

std::vector<std::string> Application::selectLayers() const {

    std::vector<std::string> requested_layers;

#ifdef VALIDATION_LAYERS
    requested_layers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif

    std::vector<vk::LayerProperties> layers = context.enumerateInstanceLayerProperties();
    std::vector<std::string> available_layers;
    for (auto &requested_layer : requested_layers) {
        bool found = false;
        for (auto &layer : layers) {
            if (strcmp(layer.layerName, requested_layer.c_str()) == 0) {
                available_layers.push_back(static_cast<const char*>(layer.layerName));
                std::cout << "Found layer: " << requested_layer << std::endl;
                found = true;
            }
        }
        if (!found) {
            std::cout << "Requested layer not found: " << std::string(requested_layer)
                << "; Expect issues" << std::endl;
        }
    }

    return available_layers;
}

std::vector<std::string> Application::selectExtensions() const {
    std::vector<vk::ExtensionProperties> extensions =
        context.enumerateInstanceExtensionProperties();

    std::vector<const char *> required_extensions =
        sf::Vulkan::getGraphicsRequiredInstanceExtensions();

#ifdef VALIDATION_LAYERS
    required_extensions.push_back(vk::EXTDebugUtilsExtensionName);
#endif
#ifdef __APPLE__
    // Add VK_KHR_PORTABILITY_subset extension for MoltenVK
    required_extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
#endif

    std::vector<std::string> available_extensions;
    for (const auto &required_extension : required_extensions) {
        bool found = false;
        for (auto &extension : extensions) {
            if (strcmp(extension.extensionName, required_extension) == 0) {
                std::cout << "Found extension: " << required_extension << std::endl;
                available_extensions.push_back(static_cast<const char*>(extension.extensionName));
                found = true;
            }
        }
        if (!found) {
            throw std::runtime_error("Required extension not found: " +
                                     std::string(required_extension));
        }
    }

    return available_extensions;
}

void Application::setupDebugMessenger() {
    constexpr vk::DebugUtilsMessengerCreateFlagsEXT create_flags;
    constexpr vk::DebugUtilsMessageSeverityFlagsEXT severity_flags =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    constexpr vk::DebugUtilsMessageTypeFlagsEXT message_type_flags =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    constexpr vk::DebugUtilsMessengerCreateInfoEXT create_info(
        create_flags, severity_flags, message_type_flags,
        debug_callback);

    debugMessenger = vk::raii::DebugUtilsMessengerEXT(instance, create_info);
}

void Application::mainLoop() {
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                return;
            }
        }
    }
}

int Application::ratePhysicalDevice(vk::raii::PhysicalDevice &physical_device,
                                      std::vector<const char *> &requested_extensions) const {
    auto properties = physical_device.getProperties();
    auto features = physical_device.getFeatures();

    std::cout << "Found Device: " << properties.deviceName << " " << properties.vendorID << " "
        << properties.deviceID << std::endl;
    std::cout << "\t"
        << "API Version: " << vk::apiVersionMajor(properties.apiVersion) << "."
        << vk::apiVersionMinor(properties.apiVersion) << "."
        << vk::apiVersionPatch(properties.apiVersion) << std::endl;
    std::cout << "\t" << "Driver Version: " << vk::apiVersionMajor(properties.driverVersion)
        << std::endl;
    std::cout << "\t" << to_string(properties.deviceType) << std::endl;

    int score = 0;

    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;
    }

    score += static_cast<int>(properties.limits.maxImageDimension2D);

    QueueFamilyIndices queue_families(physical_device, surface);

    if (!(queue_families.isComplete() && features.geometryShader &&
          check_device_extensions(physical_device, requested_extensions))) {
        return 0;
    }

    return score;
}

void Application::selectPhysicalDevice(std::vector<const char *> &requested_extensions) {
    std::vector<vk::raii::PhysicalDevice> physical_devices = instance.enumeratePhysicalDevices();
    if (physical_devices.empty()) {
        throw std::runtime_error("No physical devices found");
    }

    int max = 0;

    for (auto &item : physical_devices) {
        if (const int rating = ratePhysicalDevice(item, requested_extensions); rating > max) {
            physicalDevice = item;
            max = rating;
        }
    }

    if (physicalDevice == nullptr)
        throw std::runtime_error("No suitable physical device found");

    std::cout << "Selected Device: " << physicalDevice.getProperties().deviceName << std::endl;
}

void Application::createLogicalDevice(const std::vector<const char *> &layers,
                                        const std::vector<const char *> &extensions) {
    auto indices = QueueFamilyIndices(physicalDevice, surface);
    std::set unique_queue_indices = {indices.graphicsFamily.value(),
                                     indices.presentFamily.value()};

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    for (uint32_t queue_family_index : unique_queue_indices) {
        float queue_priority = 1.0f;
        vk::DeviceQueueCreateInfo queue_create_info(vk::DeviceQueueCreateFlags(),
                                                    queue_family_index, 1, &queue_priority);
        queue_create_infos.push_back(queue_create_info);
    }

    auto enabled_features = vk::PhysicalDeviceFeatures();
    enabled_features.geometryShader = vk::True;

    vk::DeviceCreateInfo device_create_info(vk::DeviceCreateFlags(), queue_create_infos, layers,
                                            extensions, &enabled_features);

    device = vk::raii::Device(physicalDevice, device_create_info);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
}

void Application::createSurface() {
    VkSurfaceKHR vk_surface;
    if (!window.createVulkanSurface(*instance, vk_surface))
        throw std::runtime_error("Failed to create Vulkan surface");
    surface = vk::raii::SurfaceKHR(instance, vk_surface);
}