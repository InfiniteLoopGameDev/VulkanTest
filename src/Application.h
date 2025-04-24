#pragma once

#include <SFML/Window.hpp>

#include <vulkan/vulkan_raii.hpp>

#include "ApplicationQueueFamilies.h"
#include "ApplicationSwapChainDetails.h"

class Application {
  public:
    Application();

    ~Application();

    void run();

  private:
    int maxFramesInFlight = 2;
    int currentFrame = 0;

    bool framebufferResized = false;

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

    static constexpr auto debug_utils_messenger_create_info = vk::DebugUtilsMessengerCreateInfoEXT(
        {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback);

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

    void selectPhysicalDevice(std::vector<std::string_view> &requested_extensions);

    void createSurface();

    void createLogicalDevice(const std::vector<std::string_view> &layers,
                             const std::vector<std::string_view> &extensions);

    int ratePhysicalDevice(vk::raii::PhysicalDevice &physical_device,
                           std::vector<std::string_view> &requested_extensions) const;

    void createSwapChain();

    void recreateSwapChain();

    void createImageViews();

    void createRenderPass();

    void createGraphicsPipeline();

    void createFramebuffers();

    void createCommandPool();
    void createCommandBuffers();

    void recordCommandBuffer(vk::raii::CommandBuffer &commandBuffer, uint32_t image_index);

    void drawFrame();

    void createSyncObjects();
};
