#pragma once

#include <SFML/Window.hpp>

#include <vulkan/vulkan_raii.hpp>

#include "QueueFamilyIndicies.h"

class Application {
  public:
    Application();

    ~Application();

    void run();

  private:
    sf::WindowBase window;
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::SurfaceKHR surface = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;
    vk::raii::Queue graphicsQueue = nullptr;

    void initVulkan();

    void create_instance(const std::vector<const char *> &layers,
                         const std::vector<const char *> &extensions);

    std::vector<std::string> select_layers();

    std::vector<std::string> select_extensions();

    void setupDebugMessenger();

    static unsigned int
    debugCallback([[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  vk::DebugUtilsMessageTypeFlagsEXT messageType,
                  const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

    void mainLoop();

    void select_physical_device(std::vector<const char *> &requested_extensions);

    void create_surface();

    void create_logical_device(const std::vector<const char *> &layers,
                               const std::vector<const char *> &extensions);

    int rate_physical_device(vk::raii::PhysicalDevice &physical_device,
                             std::vector<const char *> &requested_extensions);

    QueueFamilyIndices find_queue_families(vk::raii::PhysicalDevice &physical_device);
};
