#pragma once

#include <SFML/Window.hpp>

#include <vulkan/vulkan_raii.hpp>

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

    void createInstance(const std::vector<const char *> &layers,
                         const std::vector<const char *> &extensions);

    [[nodiscard]] std::vector<std::string> selectLayers() const;

    [[nodiscard]] std::vector<std::string> selectExtensions() const;

    void setupDebugMessenger();

    void mainLoop();

    void selectPhysicalDevice(std::vector<const char *> &requested_extensions);

    void createSurface();

    void createLogicalDevice(const std::vector<const char *> &layers,
                               const std::vector<const char *> &extensions);

    int ratePhysicalDevice(vk::raii::PhysicalDevice &physical_device,
                             std::vector<const char *> &requested_extensions) const;
};
