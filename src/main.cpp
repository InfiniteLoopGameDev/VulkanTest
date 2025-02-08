#include <exception>
#include <iostream>

#include <SFML/Window.hpp>

#include <vulkan/vulkan.hpp>

int main(int argv, char **args)
{
    try
    {
        sf::WindowBase window(sf::VideoMode(1280, 720), "Triangle", sf::Style::Close);

        std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();
        std::vector<const char *> requiredExtensions = sf::Vulkan::getGraphicsRequiredInstanceExtensions();
        std::vector<const char *> enabledExtensions;
        for (auto &requiredExtension : requiredExtensions)
        {
            bool found = false;
            for (auto &extension : extensions)
            {
                if (strcmp(extension.extensionName, requiredExtension) == 0)
                {
                    std::cout << "Found extension: " << requiredExtension << std::endl;
                    enabledExtensions.push_back(extension.extensionName);
                    found = true;
                }
            }
            if (!found)
            {
                throw std::runtime_error("Required extension not found: " + std::string(requiredExtension));
            }
        }

#ifndef NDEBUG // CMake only sets NDEBUG on Release builds
        std::vector<std::string> requestedLayers = {"VK_LAYER_KHRONOS_validation"};
#else
        std::vector<std::string> requestedLayers;
#endif

        // std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

        std::vector<vk::LayerProperties> layers = vk::enumerateInstanceLayerProperties();
        std::vector<const char *> enabledLayers;
        for (auto &requestedLayer : requestedLayers)
        {
            bool found = false;
            for (auto &layer : layers)
            {
                if (strcmp(layer.layerName, requestedLayer.c_str()) == 0)
                {
                    enabledLayers.push_back(layer.layerName);
                    std::cout << "Found layer: " << requestedLayer << std::endl;
                    found = true;
                }
            }
            if (!found)
            {
                throw std::runtime_error("Requested layer not found: " + std::string(requestedLayer));
            }
        }

        const vk::ApplicationInfo applicationInfo("Triangle", vk::makeApiVersion(0, 0, 1, 0), "No Engine",
                                                  vk::makeApiVersion(0, 0, 1, 0), VK_API_VERSION_1_2);

        auto instanceFlags = vk::InstanceCreateFlags();

#ifdef __APPLE__
        // Add VK_KHR_PORTABILITY_subset extension for MoltenVK
        requiredExtensions.emplace_back(vk::KHRPortabilityEnumerationExtensionName);
        instanceFlags |= vk::InstanceCreateFlagBits(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif

        vk::InstanceCreateInfo instanceInfo(instanceFlags, &applicationInfo,
                                            static_cast<uint32_t>(enabledLayers.size()), enabledLayers.data(),
                                            static_cast<uint32_t>(enabledExtensions.size()), enabledExtensions.data());

        vk::Instance instance = vk::createInstance(instanceInfo);

        vk::SurfaceKHR surface;
        VkSurfaceKHR vkSurface;
        if (!window.createVulkanSurface(instance, vkSurface))
            throw std::runtime_error("Failed to create Vulkan surface");
        surface = vk::SurfaceKHR(vkSurface);

        std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.empty())
        {
            throw std::runtime_error("No physical devices found");
        }
        for (auto &physicalDevice : physicalDevices)
        {
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

        while (true)
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    return EXIT_SUCCESS;
                }
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
