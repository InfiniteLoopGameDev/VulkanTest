#include <iostream>
#include <set>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

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

    auto layers = select_layers();
    std::vector<char const *> c_layers;
    c_layers.reserve(layers.size());
    for (auto &layer : layers) {
        c_layers.push_back(layer.c_str());
    }

    auto layer_extensions = select_extensions();
    std::vector<const char *> c_layer_extensions;
    c_layer_extensions.reserve(layer_extensions.size());
    for (auto &extension : layer_extensions) {
        c_layer_extensions.push_back(extension.c_str());
    }

    create_instance(c_layers, c_layer_extensions);

#ifdef VALIDATION_LAYERS
    setupDebugMessenger();
#endif

    create_surface();

    std::vector<const char *> c_device_extensions = {vk::KHRSwapchainExtensionName};
    select_physical_device(c_device_extensions);

    create_logical_device(c_layers, c_device_extensions);
}

void Application::create_instance(const std::vector<const char *> &layers,
                                  const std::vector<const char *> &extensions) {
    auto vk_version = vk::enumerateInstanceVersion();
    std::cout << "Vulkan version: " << vk::apiVersionMajor(vk_version) << "."
              << vk::apiVersionMinor(vk_version) << std::endl;

    const vk::ApplicationInfo applicationInfo("Triangle", vk::makeApiVersion(0, 0, 1, 0),
                                              "No Engine", vk::makeApiVersion(0, 0, 1, 0),
                                              vk::ApiVersion12);

    auto instance_flags = vk::InstanceCreateFlags();

#ifdef __APPLE__
    // Add VK_KHR_PORTABILITY_subset extension for MoltenVK
    instance_flags |= vk::raii::InstanceCreateFlagBits(
        vk::raii::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif

    vk::InstanceCreateInfo instanceInfo(instance_flags, &applicationInfo, layers, extensions);

    instance = vk::raii::Instance(context, instanceInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(static_cast<vk::Instance>(*instance));
}

std::vector<std::string> Application::select_layers() {

    std::vector<std::string> requested_layers;

#ifdef VALIDATION_LAYERS
    requested_layers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif

    std::vector<vk::LayerProperties> layers = context.enumerateInstanceLayerProperties();
    std::vector<std::string> availableLayers;
    for (auto &requestedLayer : requested_layers) {
        bool found = false;
        for (auto &layer : layers) {
            if (strcmp(layer.layerName, requestedLayer.c_str()) == 0) {
                availableLayers.push_back(static_cast<const char*>(layer.layerName));
                std::cout << "Found layer: " << requestedLayer << std::endl;
                found = true;
            }
        }
        if (!found) {
            std::cout << "Requested layer not found: " << std::string(requestedLayer)
                      << "; Expect issues" << std::endl;
        }
    }

    return availableLayers;
}

std::vector<std::string> Application::select_extensions() {
    std::vector<vk::ExtensionProperties> extensions =
        context.enumerateInstanceExtensionProperties();

    std::vector<const char *> requiredExtensions =
        sf::Vulkan::getGraphicsRequiredInstanceExtensions();

#ifdef VALIDATION_LAYERS
    requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
#endif
#ifdef __APPLE__
    // Add VK_KHR_PORTABILITY_subset extension for MoltenVK
    requiredExtensions.emplace_back(vk::raii::KHRPortabilityEnumerationExtensionName);
#endif

    std::vector<std::string> availableExtensions;
    for (auto &requiredExtension : requiredExtensions) {
        bool found = false;
        for (auto &extension : extensions) {
            if (strcmp(extension.extensionName, requiredExtension) == 0) {
                std::cout << "Found extension: " << requiredExtension << std::endl;
                availableExtensions.push_back(static_cast<const char*>(extension.extensionName));
                found = true;
            }
        }
        if (!found) {
            throw std::runtime_error("Required extension not found: " +
                                     std::string(requiredExtension));
        }
    }

    return availableExtensions;
}

void Application::setupDebugMessenger() {
    vk::DebugUtilsMessengerCreateFlagsEXT createFlags;
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    vk::DebugUtilsMessengerCreateInfoEXT createInfo(createFlags, severityFlags, messageTypeFlags,
                                                    debugCallback);

    debugMessenger = vk::raii::DebugUtilsMessengerEXT(instance, createInfo);
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL Application::debugCallback(
    [[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, [[maybe_unused]] void *pUserData) {

    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return vk::False;
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

QueueFamilyIndices Application::find_queue_families(vk::raii::PhysicalDevice &physical_device) {
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> properties = physical_device.getQueueFamilyProperties();
    int i = 0;
    for (auto property : properties) {
        if (property.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }
        if (physical_device.getSurfaceSupportKHR(i, *surface)) {
            indices.presentFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }

    return indices;
}

int Application::rate_physical_device(vk::raii::PhysicalDevice &physical_device,
                                      std::vector<const char *> &requested_extensions) {
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

    auto queue_families = find_queue_families(physical_device);

    if (!(queue_families.isComplete() & features.geometryShader &
          check_device_extensions(physical_device, requested_extensions))) {
        return 0;
    }

    return score;
}

void Application::select_physical_device(std::vector<const char *> &requested_extensions) {
    std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
    if (physicalDevices.empty()) {
        throw std::runtime_error("No physical devices found");
    }

    int max = 0;

    for (auto &item : physicalDevices) {
        int rating = rate_physical_device(item, requested_extensions);
        if (rating > max) {
            physicalDevice = item;
            max = rating;
        }
    }

    if (physicalDevice == nullptr)
        throw std::runtime_error("No suitable physical device found");

    std::cout << "Selected Device: " << physicalDevice.getProperties().deviceName << std::endl;
}

void Application::create_logical_device(const std::vector<const char *> &layers,
                                        const std::vector<const char *> &extensions) {
    auto indices = find_queue_families(physicalDevice);
    std::set<uint32_t> unique_queue_indices = {indices.graphicsFamily.value(),
                                               indices.presentFamily.value()};

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    for (uint32_t queue_family_index : unique_queue_indices) {
        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queue_create_info(vk::DeviceQueueCreateFlags(),
                                                    queue_family_index, 1, &queuePriority);
        queueCreateInfos.push_back(queue_create_info);
    }

    auto enabled_features = vk::PhysicalDeviceFeatures();
    enabled_features.geometryShader = vk::True;

    vk::DeviceCreateInfo device_create_info(vk::DeviceCreateFlags(), queueCreateInfos, layers,
                                            extensions, &enabled_features);

    device = vk::raii::Device(physicalDevice, device_create_info);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(static_cast<vk::Device>(*device));
}

void Application::create_surface() {
    VkSurfaceKHR _vkSurface;
    if (!window.createVulkanSurface(static_cast<vk::Instance>(*instance), _vkSurface))
        throw std::runtime_error("Failed to create Vulkan surface");
    surface = vk::raii::SurfaceKHR(instance, _vkSurface);
}