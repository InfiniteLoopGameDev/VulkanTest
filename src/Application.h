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

    void createInstance(const std::vector<std::string_view> &layers,
                        const std::vector<std::string_view> &extensions);

    [[nodiscard]] std::vector<std::string_view> selectLayers() const;

    [[nodiscard]] std::vector<std::string_view> selectExtensions() const;

    void setupDebugMessenger();

    void mainLoop();

    void selectPhysicalDevice(std::vector<std::string_view> &requested_extensions);

    void createSurface();

    void createLogicalDevice(const std::vector<std::string_view> &layers,
                             const std::vector<std::string_view> &extensions);

    int ratePhysicalDevice(vk::raii::PhysicalDevice &physical_device,
                           std::vector<std::string_view> &requested_extensions) const;
};
