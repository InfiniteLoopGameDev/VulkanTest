#include <fstream>
#include <iostream>
#include <set>

#include "Application.h"

#include "QueueFamilyIndicies.h"
#include "SwapChainDetails.h"

#include "utils.h"

#ifndef NDEBUG
#define VALIDATION_LAYERS // CMake only sets NDEBUG on Release builds
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

Application::Application() {
    window.create(sf::VideoMode({1280, 720}), "Triangle", sf::Style::Close);
    initVulkan();
}

Application::~Application() = default;

void Application::run() { mainLoop(); }

void Application::initVulkan() {
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
    createCommandBuffer();

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
    instance_flags |= vk::raii::InstanceCreateFlagBits(
        vk::raii::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif

    auto c_extensions = to_c_strings(extensions);
    auto c_layers = to_c_strings(layers);

    const vk::InstanceCreateInfo instance_info(instance_flags, &application_info, c_layers,
                                               c_extensions);

    instance = vk::raii::Instance(context, instance_info);
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

void Application::createRenderPass() {
    const vk::AttachmentDescription color_attachments(
        {}, swapChainImageFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR);

    constexpr vk::AttachmentReference color_attachment(0, vk::ImageLayout::eColorAttachmentOptimal);
    const vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1,
                                         &color_attachment);

    const vk::SubpassDependency subpass_dependency(
        vk::SubpassExternal, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput, {},
        vk::AccessFlagBits::eColorAttachmentWrite);

    const vk::RenderPassCreateInfo render_pass_info({}, 1, &color_attachments, 1, &subpass, 1,
                                                    &subpass_dependency);

    renderPass = vk::raii::RenderPass(device, render_pass_info);
}

void Application::createGraphicsPipeline() {
    std::ifstream file("triangle.spv", std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    auto file_size = file.tellg();
    std::vector<char> shader(file_size);

    file.seekg(0);
    file.read(shader.data(), file_size);
    file.close();

    vk::ShaderModuleCreateInfo shader_module_create_info(
        vk::ShaderModuleCreateFlags(), file_size,
        reinterpret_cast<const uint32_t *>(shader.data()));

    vk::raii::ShaderModule shader_module(device, shader_module_create_info);

    vk::PipelineShaderStageCreateInfo vertex_stage_info(vk::PipelineShaderStageCreateFlags(),
                                                        vk::ShaderStageFlagBits::eVertex,
                                                        *shader_module, "vertexMain", nullptr);

    vk::PipelineShaderStageCreateInfo fragment_stage_info(vk::PipelineShaderStageCreateFlags(),
                                                          vk::ShaderStageFlagBits::eFragment,
                                                          *shader_module, "fragmentMain", nullptr);

    vk::PipelineShaderStageCreateInfo shader_stages[] = {vertex_stage_info, fragment_stage_info};

    vk::PipelineVertexInputStateCreateInfo vertex_input_info(
        vk::PipelineVertexInputStateCreateFlags(), 0, nullptr, 0, nullptr);

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_info(
        vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, false);

    vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width),
                          static_cast<float>(swapChainExtent.height), 0.0f, 1.0f);

    vk::Rect2D scissor(vk::Offset2D(), swapChainExtent);

    vk::PipelineViewportStateCreateInfo viewport_state_info(vk::PipelineViewportStateCreateFlags(),
                                                            1, &viewport, 1, &scissor);

    vk::PipelineRasterizationStateCreateInfo rasterization_info(
        {}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f);

    vk::PipelineMultisampleStateCreateInfo multisampling_info({}, vk::SampleCountFlagBits::e1,
                                                              false, 1.0f, nullptr, false, false);

    vk::PipelineColorBlendAttachmentState color_blend_attachment(
        false, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB);

    vk::PipelineColorBlendStateCreateInfo color_blend_state(
        vk::PipelineColorBlendStateCreateFlags(), false, vk::LogicOp::eCopy, 1,
        &color_blend_attachment, {0.0f, 0.0f, 0.0f, 0.0f});

    vk::PipelineLayoutCreateInfo pipeline_layout_info{};

    pipelineLayout = vk::raii::PipelineLayout(device, pipeline_layout_info);
    vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info(
        {}, 2, shader_stages, &vertex_input_info, &input_assembly_info, nullptr,
        &viewport_state_info, &rasterization_info, &multisampling_info, nullptr, &color_blend_state,
        nullptr, pipelineLayout, renderPass, 0);

    graphicsPipeline = vk::raii::Pipeline(device, nullptr, graphics_pipeline_create_info);
}

void Application::createFramebuffers() {
    swapChainFramebuffers.reserve(swapChainImageViews.size());
    for (auto &image_views : swapChainImageViews) {
        vk::ImageView attachments[] = {image_views};

        vk::FramebufferCreateInfo framebuffer_info(
            {}, *renderPass, 1, attachments, swapChainExtent.width, swapChainExtent.height, 1);

        swapChainFramebuffers.emplace_back(device, framebuffer_info);
    }
}

void Application::createCommandPool() {
    QueueFamilyIndices queue_family_indices(physicalDevice, surface);
    vk::CommandPoolCreateInfo pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                        queue_family_indices.graphicsFamily.value());

    commandPool = vk::raii::CommandPool(device, pool_info);
}

void Application::createCommandBuffer() {
    vk::CommandBufferAllocateInfo command_buffer_allocate_info(commandPool,
                                                               vk::CommandBufferLevel::ePrimary, 1);

    commandBuffer = std::move(vk::raii::CommandBuffers(device, command_buffer_allocate_info)[0]);
}

void Application::recordCommandBuffer(uint32_t image_index) {
    vk::CommandBufferBeginInfo begin_info{};
    commandBuffer.begin(begin_info);

    constexpr vk::ClearValue clear_color_value(
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));
    vk::RenderPassBeginInfo render_pass_info(renderPass, swapChainFramebuffers[image_index],
                                             vk::Rect2D({}, swapChainExtent), 1,
                                             &clear_color_value);
    commandBuffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    commandBuffer.end();
}

void Application::createSyncObjects() {

    imageAvailableSemaphore = device.createSemaphore({});
    renderFinishedSemaphore = device.createSemaphore({});
    inFlightFence = vk::raii::Fence(device, {vk::FenceCreateFlagBits::eSignaled});
}

void Application::drawFrame() {
    while (vk::Result::eTimeout == device.waitForFences({inFlightFence}, true, UINT64_MAX))
        ;

    device.resetFences({inFlightFence});

    auto [result, image_index] =
        swapChain.acquireNextImage(UINT64_MAX, imageAvailableSemaphore, nullptr);
    assert(result == vk::Result::eSuccess);

    commandBuffer.reset();
    recordCommandBuffer(image_index);

    const std::array<vk::Semaphore, 1> wait_semaphores{*imageAvailableSemaphore};
    const std::array<vk::Semaphore, 1> signal_semaphores{*renderFinishedSemaphore};
    constexpr std::array<vk::PipelineStageFlags, 1> wait_stages{
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    const std::array<vk::CommandBuffer, 1> command_buffers{*commandBuffer};

    vk::SubmitInfo submit_info(wait_semaphores, wait_stages, command_buffers, signal_semaphores);

    graphicsQueue.submit(submit_info, inFlightFence);

    vk::PresentInfoKHR present_info(signal_semaphores, *swapChain, image_index);
    result = graphicsQueue.presentKHR(present_info);
    assert(result == vk::Result::eSuccess);
}