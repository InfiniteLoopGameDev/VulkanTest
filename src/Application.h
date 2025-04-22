#pragma once

#include <SFML/Window.hpp>

#include <vulkan/vulkan_raii.hpp>

class Application {
  public:
    Application();

    ~Application();

    void run();

  private:
    int maxFramesInFlight = 2;
    int currentFrame = 0;

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

    void createRenderPass();

    void createGraphicsPipeline();

    void createFramebuffers();

    void createCommandPool();
    void createCommandBuffers();

    void recordCommandBuffer(vk::raii::CommandBuffer &commandBuffer, uint32_t image_index);

    void drawFrame();

    void createSyncObjects();
};
