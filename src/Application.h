#ifndef VULKANTEST_APPLICATION_H
#define VULKANTEST_APPLICATION_H

#include <SFML/Window.hpp>

#include <vulkan/vulkan.hpp>

class Application {
public:
    Application();

    ~Application();

    void run();

private:
    sf::WindowBase window = sf::WindowBase(sf::VideoMode(1280, 720), "Triangle", sf::Style::Close);
    vk::Instance instance;
    vk::SurfaceKHR surface;
    vk::DebugUtilsMessengerEXT debugMessenger;

    void initVulkan();

    void create_instance(const std::vector<std::string> &layers, const std::vector<std::string> &extensions);

    static std::vector<std::string> select_layers();

    static std::vector<std::string> select_extensions();

    void setupDebugMessenger();

    static unsigned int
    debugCallback([[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  vk::DebugUtilsMessageTypeFlagsEXT messageType,
                  const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

    void mainLoop();

    void select_physical_device();

    void create_surface();
};


#endif //VULKANTEST_APPLICATION_H
