#pragma once

#include <SFML/Window.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "ApplicationQueueFamilies.h"
#include "ApplicationSwapChainDetails.h"

// TODO: UBO -> HDR Color-space: conversion
// TODO: Switch to SDL or GLFW / SFML no OpenGL patch?
// TODO: ~Use designated initializers~ seems to be complicated with ArrayNoProxies
// TODO: Use VK_KHR_display for monitor selection and fullscreen logic
// TODO: Combine vertex and index into one (with offsets)

class Application {
public:
    Application();

    ~Application();

    void run();

private:
    unsigned int maxFramesInFlight = 2;
    unsigned int currentFrame = 0;

    unsigned int swapChainImageCount = 1;
    unsigned int currentSwapChainImage = 0;

    unsigned int frameCount = 0;
    sf::Clock timer;

    sf::WindowBase window;
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::SurfaceKHR surface = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;

    // Synchronization objects need to be destroyed after the Queue
    std::vector<vk::raii::Semaphore> imageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence> inFlightFences;

    vk::raii::Queue graphicsQueue = nullptr;
    vk::raii::SwapchainKHR swapChain = nullptr;
    vk::raii::RenderPass renderPass = nullptr;
    vk::raii::PipelineLayout pipelineLayout = nullptr;
    vk::raii::Pipeline graphicsPipeline = nullptr;
    vk::raii::CommandPool commandPool = nullptr;
    vk::raii::Buffer vertexBuffer = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
    vk::raii::Buffer indexBuffer = nullptr;
    vk::raii::DeviceMemory indexBufferMemory = nullptr;
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::raii::ImageView> swapChainImageViews;
    std::vector<vk::raii::Framebuffer> swapChainFramebuffers;

    ApplicationQueueFamilies queueFamilies;
    ApplicationSwapChainDetails swapChainDetails;

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL
    debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
                  vk::DebugUtilsMessageTypeFlagsEXT message_type,
                  const vk::DebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

    static constexpr vk::DebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{
        {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback};

    static bool checkDeviceExtensions(const vk::raii::PhysicalDevice &device,
                                      const std::vector<std::string_view> &requested_extensions);

    [[nodiscard]] static vk::SurfaceFormatKHR
    chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats);

    [[nodiscard]] static vk::PresentModeKHR
    choosePresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes,
                      const std::vector<vk::PresentModeKHR> &present_mode_preferences);

    [[nodiscard]] vk::Extent2D
    chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const;

    void initVulkan();

    void createInstance(const std::vector<std::string_view> &layers,
                        const std::vector<std::string_view> &extensions);

    [[nodiscard]] std::vector<std::string_view> selectLayers() const;

    [[nodiscard]] std::vector<std::string_view> selectExtensions() const;

    void setupDebugMessenger();

    void mainLoop();

    void selectPhysicalDevice(const std::vector<std::string_view> &requested_extensions);

    void createSurface();

    void createLogicalDevice(const std::vector<std::string_view> &layers,
                             const std::vector<std::string_view> &extensions);

    [[nodiscard]] int
    ratePhysicalDevice(const vk::raii::PhysicalDevice &physical_device,
                       const std::vector<std::string_view> &requested_extensions) const;

    void createSwapChain();

    void recreateSwapChain();

    void createImageViews();

    void createRenderPass();

    void createGraphicsPipeline();

    void createFramebuffers();

    void createCommandPool();

    void copyBuffer(const vk::raii::Buffer &src_buffer, vk::raii::Buffer &dst_buffer, vk::DeviceSize size);

    [[nodiscard]] std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(
        vk::DeviceSize size, vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties) const;

    void createVertexBuffer();
    void createIndexBuffer();

    void createCommandBuffers();

    void recordCommandBuffer(const vk::raii::CommandBuffer &command_buffer,
                             uint32_t image_index) const;

    void drawFrame();

    void createSyncObjects();
};
