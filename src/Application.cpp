#include <iostream>

// Enables the built-in dispatch loader
// Allows to use extensions like VK_EXT_debug_utils

#include "Application.h"

#ifndef NDEBUG
#define VALIDATION_LAYERS // CMake only sets NDEBUG on Release builds
#endif

Application::Application() {
    initVulkan();
}

Application::~Application() = default;

void Application::run() {
    mainLoop();
}

void Application::initVulkan() {
    auto layers = select_layers();
    auto extensions = select_extensions();
    create_instance(layers, extensions);

#ifdef VALIDATION_LAYERS
    setupDebugMessenger();
#endif

    create_surface();

    select_physical_device();
}

void Application::create_instance(const std::vector<std::string> &layers, const std::vector<std::string> &extensions) {
    const vk::ApplicationInfo applicationInfo("Triangle", vk::makeApiVersion(0, 0, 1, 0), "No Engine",
                                              vk::makeApiVersion(0, 0, 1, 0), vk::ApiVersion12);

    auto instanceFlags = vk::InstanceCreateFlags();

#ifdef __APPLE__
    // Add VK_KHR_PORTABILITY_subset extension for MoltenVK
    instanceFlags |= vk::raii::InstanceCreateFlagBits(vk::raii::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif

    std::vector<char const *> c_layers;
    c_layers.reserve(layers.size());
    for (auto &layer: layers) {
        c_layers.push_back(layer.c_str());
    }

    std::vector<const char *> c_extensions;
    c_extensions.reserve(extensions.size());
    for (auto &extension: extensions) {
        c_extensions.push_back(extension.c_str());
    }

    vk::InstanceCreateInfo instanceInfo(instanceFlags, &applicationInfo,
                                        static_cast<uint32_t>(c_layers.size()), c_layers.data(),
                                        static_cast<uint32_t>(c_extensions.size()), c_extensions.data());

    instance = vk::raii::Instance(context, instanceInfo);
}

std::vector<std::string> Application::select_layers() {

#ifdef VALIDATION_LAYERS
    std::vector<std::string> requestedLayers = {"VK_LAYER_KHRONOS_validation"};
#else
    std::vector<std::string> requestedLayers;
#endif

    std::vector<vk::LayerProperties> layers = context.enumerateInstanceLayerProperties();
    std::vector<std::string> availableLayers;
    for (auto &requestedLayer: requestedLayers) {
        bool found = false;
        for (auto &layer: layers) {
            if (strcmp(layer.layerName, requestedLayer.c_str()) == 0) {
                availableLayers.push_back(std::string(layer.layerName));
                std::cout << "Found layer: " << requestedLayer << std::endl;
                found = true;
            }
        }
        if (!found) {
            std::cout << "Requested layer not found: " << std::string(requestedLayer) << "; Expect issues" << std::endl;
        }
    }

    return availableLayers;
}

std::vector<std::string> Application::select_extensions() {
    std::vector<vk::ExtensionProperties> extensions = context.enumerateInstanceExtensionProperties();

    std::vector<const char *> requiredExtensions = sf::Vulkan::getGraphicsRequiredInstanceExtensions();

#ifdef VALIDATION_LAYERS
    requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
#endif
#ifdef __APPLE__
    // Add VK_KHR_PORTABILITY_subset extension for MoltenVK
    requiredExtensions.emplace_back(vk::raii::KHRPortabilityEnumerationExtensionName);
#endif

    std::vector<std::string> availableExtensions;
    for (auto &requiredExtension: requiredExtensions) {
        bool found = false;
        for (auto &extension: extensions) {
            if (strcmp(extension.extensionName, requiredExtension) == 0) {
                std::cout << "Found extension: " << requiredExtension << std::endl;
                availableExtensions.push_back(std::string(extension.extensionName));
                found = true;
            }
        }
        if (!found) {
            throw std::runtime_error("Required extension not found: " + std::string(requiredExtension));
        }
    }

    return availableExtensions;
}

void Application::setupDebugMessenger() {
    vk::DebugUtilsMessengerCreateFlagsEXT createFlags;
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                          vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                         vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                         vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    vk::DebugUtilsMessengerCreateInfoEXT createInfo(createFlags, severityFlags, messageTypeFlags, debugCallback);

    debugMessenger = vk::raii::DebugUtilsMessengerEXT(instance, createInfo);
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL Application::debugCallback(
        [[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        [[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
        [[maybe_unused]] void *pUserData) {

    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return vk::False;
}

void Application::mainLoop() {
    while (true) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return;
            }
        }
    }
}

void Application::select_physical_device() {
    std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
    if (physicalDevices.empty()) {
        throw std::runtime_error("No physical devices found");
    }
    for (auto &physicalDevice: physicalDevices) {
        vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
        auto features = physicalDevice.getFeatures();
        std::cout << "Device: " << properties.deviceName << " " << properties.vendorID << " " << properties.deviceID
                  << std::endl;
        std::cout << "\t" << "API Version: " << vk::apiVersionMajor(properties.apiVersion) << "."
                  << vk::apiVersionMinor(properties.apiVersion) << "." << vk::apiVersionPatch(properties.apiVersion)
                  << std::endl;
        std::cout << "\t" << "Driver Version: " << vk::apiVersionMajor(properties.driverVersion) << std::endl;
        std::cout << "\t" << to_string(properties.deviceType) << std::endl;
    }
}

void Application::create_surface() {
    VkSurfaceKHR _vkSurface;
    if (!window.createVulkanSurface(static_cast<VkInstance>( *instance ), _vkSurface))
        throw std::runtime_error("Failed to create Vulkan surface");
    surface = vk::raii::SurfaceKHR(instance, _vkSurface);
}