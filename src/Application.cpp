#include <fstream>
#include <iostream>
#include <set>

#include "Application.h"

#include "QueueFamilyIndices.h"
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
    assert(sf::Vulkan::isAvailable()); // TODO: Proper runtime check of Vulkan availability

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

    createRenderPass();

    createGraphicsPipeline();

    createFramebuffers();

    createCommandPool();
    createCommandBuffers();

    createSyncObjects();
}

void Application::createInstance(const std::vector<std::string_view> &layers,
                                 const std::vector<std::string_view> &extensions) {
    const auto vk_version = context.enumerateInstanceVersion();
    std::cout << "Vulkan version: " << vk::apiVersionMajor(vk_version) << "."
              << vk::apiVersionMinor(vk_version) << std::endl;

    constexpr vk::ApplicationInfo application_info("Triangle", vk::makeApiVersion(0, 0, 1, 0),
                                                   "No Engine", vk::makeApiVersion(0, 0, 1, 0),
                                                   vk::ApiVersion12);

    auto instance_flags = vk::InstanceCreateFlags();

#ifdef __APPLE__
    // Add VK_KHR_PORTABILITY_subset extension for MoltenVK
    instance_flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    auto c_extensions = to_c_strings(extensions);
    auto c_layers = to_c_strings(layers);

    const vk::InstanceCreateInfo instance_info(instance_flags, &application_info, c_layers,
                                               c_extensions);

    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
        instance_info_chain(instance_info, debugUtilsMessengerCreateInfo);

    instance = context.createInstance(instance_info_chain.get<vk::InstanceCreateInfo>());
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
    debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfo);
}

void Application::mainLoop() {
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                device.waitIdle();
                return;
            }
        }
        drawFrame();
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

    // TODO: Use std::max_element with custom comparator
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
        auto queue_priority = {1.0f};
        vk::DeviceQueueCreateInfo queue_create_info({}, queue_family_index, queue_priority);
        queue_create_infos.push_back(queue_create_info);
    }

    vk::PhysicalDeviceFeatures enabled_features;
    enabled_features.geometryShader = true;

    auto c_layers = to_c_strings(layers);
    auto c_extensions = to_c_strings(extensions);

    vk::DeviceCreateInfo device_create_info({}, queue_create_infos, c_layers, c_extensions,
                                            &enabled_features);

    device = physicalDevice.createDevice(device_create_info);

    graphicsQueue = device.getQueue(indices[0], 0);
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

    uint32_t image_count;
    image_count = swap_chain_details.capabilities.minImageCount + 1;
    if (swap_chain_details.capabilities.maxImageCount > 0 &&
        image_count > swap_chain_details.capabilities.maxImageCount) {
        image_count = swap_chain_details.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swap_chain_create_info(
        {}, surface, image_count, surface_format.format, surface_format.colorSpace, extent, 1,
        vk::ImageUsageFlagBits::eColorAttachment);

    swap_chain_create_info.preTransform = swap_chain_details.capabilities.currentTransform;
    swap_chain_create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swap_chain_create_info.presentMode = present_mode;
    swap_chain_create_info.clipped = true;

    if (const QueueFamilyIndices indices(physicalDevice, surface); indices.areUnique()) {
        const auto queue_family_indices = indices.getQueueFamilies();
        swap_chain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swap_chain_create_info.setQueueFamilyIndices(queue_family_indices);
    } else {
        swap_chain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapChain = device.createSwapchainKHR(swap_chain_create_info);

    swapChainImageFormat = surface_format.format;
    swapChainExtent = extent;
    swapChainImages = swapChain.getImages();
}

void Application::createRenderPass() {
    const vk::AttachmentDescription color_attachments(
        {}, swapChainImageFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR);

    constexpr vk::AttachmentReference color_attachment(0, vk::ImageLayout::eColorAttachmentOptimal);
    const vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, {},
                                         color_attachment);

    const vk::SubpassDependency subpass_dependency(
        vk::SubpassExternal, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput, {},
        vk::AccessFlagBits::eColorAttachmentWrite);

    const vk::RenderPassCreateInfo render_pass_info({}, color_attachments, subpass,
                                                    subpass_dependency);

    renderPass = device.createRenderPass(render_pass_info);
}

void Application::createGraphicsPipeline() {
#include "triangle.spv.h"

    // static_assert(triangle[0] == 0x07230203, "Invalid SPIR-V magic number");

    vk::ShaderModuleCreateInfo shader_module_create_info({}, triangle_sizeInBytes, triangle);

    auto shader_module = device.createShaderModule(shader_module_create_info);

    vk::PipelineShaderStageCreateInfo vertex_stage_info({}, vk::ShaderStageFlagBits::eVertex,
                                                        shader_module, "vertexMain");

    vk::PipelineShaderStageCreateInfo fragment_stage_info({}, vk::ShaderStageFlagBits::eFragment,
                                                          shader_module, "fragmentMain");

    vk::PipelineShaderStageCreateInfo shader_stages[] = {vertex_stage_info, fragment_stage_info};

    vk::PipelineVertexInputStateCreateInfo vertex_input_info;

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_info(
        {}, vk::PrimitiveTopology::eTriangleList, false);

    vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width),
                          static_cast<float>(swapChainExtent.height), 0.0f, 1.0f);

    vk::Rect2D scissor({}, swapChainExtent);

    vk::PipelineViewportStateCreateInfo viewport_state_info({}, viewport, scissor);

    vk::PipelineRasterizationStateCreateInfo rasterization_info(
        {}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f);

    vk::PipelineMultisampleStateCreateInfo multisampling_info({}, vk::SampleCountFlagBits::e1,
                                                              false, 1.0f);

    auto color_write_mask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB;

    vk::PipelineColorBlendAttachmentState color_blend_attachment(
        false, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, color_write_mask);

    vk::PipelineColorBlendStateCreateInfo color_blend_state(
        {}, false, vk::LogicOp::eCopy, color_blend_attachment, {0.0f, 0.0f, 0.0f, 0.0f});

    pipelineLayout = device.createPipelineLayout({});
    vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info(
        {}, shader_stages, &vertex_input_info, &input_assembly_info, {}, &viewport_state_info,
        &rasterization_info, &multisampling_info, {}, &color_blend_state, {}, pipelineLayout,
        renderPass, 0);

    graphicsPipeline = device.createGraphicsPipeline(nullptr, graphics_pipeline_create_info);
}

void Application::createFramebuffers() {
    swapChainFramebuffers.reserve(swapChainImageViews.size());
    for (auto &image_views : swapChainImageViews) {
        vk::ImageView attachments[] = {image_views};

        vk::FramebufferCreateInfo framebuffer_info(
            {}, renderPass, attachments, swapChainExtent.width, swapChainExtent.height, 1);

        swapChainFramebuffers.emplace_back(device, framebuffer_info);
    }
}

void Application::createCommandPool() {
    QueueFamilyIndices queue_family_indices(physicalDevice, surface);
    vk::CommandPoolCreateInfo pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                        queue_family_indices.graphicsFamily.value());

    commandPool = device.createCommandPool(pool_info);
}

void Application::createCommandBuffers() {
    commandBuffers.reserve(maxFramesInFlight);

    vk::CommandBufferAllocateInfo command_buffer_allocate_info(
        commandPool, vk::CommandBufferLevel::ePrimary, maxFramesInFlight);

    commandBuffers = device.allocateCommandBuffers(command_buffer_allocate_info);
}

void Application::recordCommandBuffer(vk::raii::CommandBuffer &commandBuffer,
                                      uint32_t image_index) {
    commandBuffer.begin({});

    constexpr vk::ClearValue clear_color_value(
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));
    vk::RenderPassBeginInfo render_pass_info(renderPass, swapChainFramebuffers[image_index],
                                             vk::Rect2D({}, swapChainExtent), clear_color_value);

    commandBuffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    commandBuffer.end();
}

void Application::createSyncObjects() {
    imageAvailableSemaphores.reserve(maxFramesInFlight);
    renderFinishedSemaphores.reserve(maxFramesInFlight);
    inFlightFences.reserve(maxFramesInFlight);

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        imageAvailableSemaphores.emplace_back(device.createSemaphore({}));
        renderFinishedSemaphores.emplace_back(device.createSemaphore({}));
        inFlightFences.emplace_back(device.createFence({vk::FenceCreateFlagBits::eSignaled}));
    }
}

void Application::drawFrame() {

    auto &current_command_buffer = commandBuffers[currentFrame];
    auto &current_image_available_semaphore = imageAvailableSemaphores[currentFrame];
    auto &current_render_finished_semaphore = renderFinishedSemaphores[currentFrame];
    auto &current_in_flight_fence = inFlightFences[currentFrame];

    while (vk::Result::eTimeout ==
           device.waitForFences({current_in_flight_fence}, true, UINT64_MAX))
        ;

    device.resetFences({current_in_flight_fence});

    auto [result, image_index] =
        swapChain.acquireNextImage(UINT64_MAX, current_image_available_semaphore);
    assert(result == vk::Result::eSuccess);

    current_command_buffer.reset();
    recordCommandBuffer(current_command_buffer, image_index);

    constexpr std::array<vk::PipelineStageFlags, 1> wait_stages{
        vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submit_info(*current_image_available_semaphore, wait_stages,
                               *current_command_buffer, *current_render_finished_semaphore);

    graphicsQueue.submit(submit_info, current_in_flight_fence);

    vk::PresentInfoKHR present_info(*current_render_finished_semaphore, *swapChain, image_index);
    result = graphicsQueue.presentKHR(present_info);
    assert(result == vk::Result::eSuccess);

    currentFrame = (currentFrame + 1) % maxFramesInFlight;
}