#include <iostream>
#include <set>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include "Application.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include "QueueFamilyIndicies.h"
#include "SwapChainDetails.h"

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

    const auto layer_extensions = selectExtensions();

    createInstance(layers, layer_extensions);

#ifdef VALIDATION_LAYERS
    setupDebugMessenger();
#endif

    createSurface();

    std::vector<std::string_view> device_extensions = {vk::KHRSwapchainExtensionName};
    selectPhysicalDevice(device_extensions);

    createLogicalDevice(layers, device_extensions);

    createSwapChain();
    swapChainImageViews = create_image_views(device, swapChainImages, swapChainImageFormat);
}

void Application::createInstance(const std::vector<std::string_view> &layers,
                                 const std::vector<std::string_view> &extensions) {
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

    auto c_extensions = to_c_strings(extensions);
    auto c_layers = to_c_strings(layers);

    const vk::InstanceCreateInfo instance_info(instance_flags, &application_info, c_layers,
                                               c_extensions);

    instance = vk::raii::Instance(context, instance_info);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
}

std::vector<std::string_view> Application::selectLayers() const {

    std::vector<std::string_view> requested_layers;

#ifdef VALIDATION_LAYERS
    requested_layers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif

    std::vector<vk::LayerProperties> layers = context.enumerateInstanceLayerProperties();
    std::vector<std::string_view> available_layers;
    for (auto &requested_layer : requested_layers) {
        bool found = false;
        for (auto &layer : layers) {
            if (static_cast<std::string_view>(layer.layerName.data()) == requested_layer.data()) {
                available_layers.push_back(requested_layer);
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

std::vector<std::string_view> Application::selectExtensions() const {
    std::vector<vk::ExtensionProperties> extensions =
        context.enumerateInstanceExtensionProperties();

    auto sfml_extensions = sf::Vulkan::getGraphicsRequiredInstanceExtensions();
    std::vector<std::string_view> required_extensions(sfml_extensions.begin(),
                                                      sfml_extensions.end());

#ifdef VALIDATION_LAYERS
    required_extensions.emplace_back(vk::EXTDebugUtilsExtensionName);
#endif
#ifdef __APPLE__
    // Add VK_KHR_PORTABILITY_subset extension for MoltenVK
    required_extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
#endif

    std::vector<std::string_view> available_extensions;
    for (const auto &required_extension : required_extensions) {
        bool found = false;
        for (auto &extension : extensions) {
            if (static_cast<std::string_view>(extension.extensionName.data()) ==
                required_extension) {
                std::cout << "Found extension: " << required_extension << std::endl;
                available_extensions.push_back(required_extension);
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
    constexpr vk::DebugUtilsMessengerCreateInfoEXT create_info(create_flags, severity_flags,
                                                               message_type_flags, debug_callback);

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
                                    std::vector<std::string_view> &requested_extensions) const {
    auto properties = physical_device.getProperties();
    auto features = physical_device.getFeatures();

    std::cout << "Found Device: " << properties.deviceName << " " << properties.vendorID << " "
              << properties.deviceID << std::endl;
    std::cout << "\t"
              << "API Version: " << vk::apiVersionMajor(properties.apiVersion) << "."
              << vk::apiVersionMinor(properties.apiVersion) << "."
              << vk::apiVersionPatch(properties.apiVersion) << std::endl;
    std::cout << "\t" << "Driver Version: " << vk::apiVersionMajor(properties.driverVersion) << "."
              << vk::apiVersionMinor(properties.driverVersion) << "."
              << vk::apiVersionPatch(properties.driverVersion) << std::endl;
    std::cout << "\t" << to_string(properties.deviceType) << std::endl;

    int score = 0;

    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;
    }

    score += static_cast<int>(properties.limits.maxImageDimension2D);

    QueueFamilyIndices queue_families(physical_device, surface);

    SwapChainDetails swap_chain_details(physical_device, surface);

    if (!(queue_families.isComplete() && features.geometryShader &&
          check_device_extensions(physical_device, requested_extensions) &&
          swap_chain_details.isValid())) {
        return 0;
    }

    return score;
}

void Application::selectPhysicalDevice(std::vector<std::string_view> &requested_extensions) {
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

void Application::createLogicalDevice(const std::vector<std::string_view> &layers,
                                      const std::vector<std::string_view> &extensions) {
    std::vector<uint32_t> indices = QueueFamilyIndices(physicalDevice, surface).getQueueFamilies();
    std::set unique_queue_indices(indices.begin(), indices.end());

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    for (uint32_t queue_family_index : unique_queue_indices) {
        float queue_priority = 1.0f;
        vk::DeviceQueueCreateInfo queue_create_info(vk::DeviceQueueCreateFlags(),
                                                    queue_family_index, 1, &queue_priority);
        queue_create_infos.push_back(queue_create_info);
    }

    auto enabled_features = vk::PhysicalDeviceFeatures();
    enabled_features.geometryShader = vk::True;

    auto c_layers = to_c_strings(layers);
    auto c_extensions = to_c_strings(extensions);

    vk::DeviceCreateInfo device_create_info(vk::DeviceCreateFlags(), queue_create_infos, c_layers,
                                            c_extensions, &enabled_features);

    device = vk::raii::Device(physicalDevice, device_create_info);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
}

void Application::createSurface() {
    VkSurfaceKHR vk_surface;
    if (!window.createVulkanSurface(*instance, vk_surface))
        throw std::runtime_error("Failed to create Vulkan surface");
    surface = vk::raii::SurfaceKHR(instance, vk_surface);
}

void Application::createSwapChain() {
    const SwapChainDetails swap_chain_details(physicalDevice, surface);

    const vk::SurfaceFormatKHR surface_format =
        choose_swap_surface_format(swap_chain_details.formats);
    const vk::PresentModeKHR present_mode = choose_present_mode(swap_chain_details.presentModes);
    const vk::Extent2D extent =
        choose_swap_extent(swap_chain_details.capabilities, window.getSize());

    uint32_t image_count = swap_chain_details.capabilities.minImageCount + 1;
    if (swap_chain_details.capabilities.maxImageCount > 0 &&
        image_count > swap_chain_details.capabilities.maxImageCount) {
        image_count = swap_chain_details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swap_chain_create_info(
        vk::SwapchainCreateFlagsKHR(), surface, image_count, surface_format.format,
        surface_format.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment);

    swap_chain_create_info.preTransform = swap_chain_details.capabilities.currentTransform;
    swap_chain_create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swap_chain_create_info.presentMode = present_mode;
    swap_chain_create_info.clipped = true;
    swap_chain_create_info.oldSwapchain = nullptr;

    if (const QueueFamilyIndices indices(physicalDevice, surface); indices.areUnique()) {
        const auto queue_family_indices = indices.getQueueFamilies();
        swap_chain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swap_chain_create_info.queueFamilyIndexCount = queue_family_indices.size();
        swap_chain_create_info.pQueueFamilyIndices = queue_family_indices.data();
    } else {
        swap_chain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapChain = vk::raii::SwapchainKHR(device, swap_chain_create_info);

    swapChainImageFormat = surface_format.format;
    swapChainExtent = extent;
    swapChainImages = swapChain.getImages();
}
