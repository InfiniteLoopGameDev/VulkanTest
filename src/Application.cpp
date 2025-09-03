#include <iostream>
#include <set>
#include <unordered_map>

#include "Application.h"

#include "utils.h"

#ifndef NDEBUG
#define VALIDATION_LAYERS // CMake only sets NDEBUG on Release builds
#endif

[[nodiscard]] static int rate_surface_format(const vk::SurfaceFormatKHR &surface_format) {
    int total = 0;

    switch (surface_format.format) {
    case vk::Format::eA2B10G10R10UnormPack32:
        total += 500;
    case vk::Format::eB8G8R8A8Unorm:
        total += 500;
        break;
    default:
        return 0;
    }

    switch (surface_format.colorSpace) {
    case vk::ColorSpaceKHR::eHdr10St2084EXT:
        // total += 1000;
    case vk::ColorSpaceKHR::eSrgbNonlinear:
        total += 500;
        break;
    default:
        return 0;
    }

    return total;
}

Application::Application() {
    window.create(sf::VideoMode({1280, 720}), "Triangle", sf::Style::Default);
    initVulkan();
}

Application::~Application() = default;

VKAPI_ATTR vk::Bool32 VKAPI_CALL Application::debugCallback(
    [[maybe_unused]] vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
    [[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT message_type,
    const vk::DebugUtilsMessengerCallbackDataEXT *callback_data, [[maybe_unused]] void *user_data) {

    std::cerr << "Validation layer: " << callback_data->pMessage << std::endl;

    return vk::False;
}

bool Application::checkDeviceExtensions(const vk::raii::PhysicalDevice &device,
                                        const std::vector<std::string_view> &requested_extensions) {
    const std::vector<vk::ExtensionProperties> available_extensions =
        device.enumerateDeviceExtensionProperties();
    std::set<std::string> required_extensions(requested_extensions.begin(),
                                              requested_extensions.end());

    for (auto &extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

vk::SurfaceFormatKHR
Application::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats) {
    int max = 0;
    vk::SurfaceFormatKHR best_format;

    for (auto &available_format : available_formats) {
        if (const int rating = rate_surface_format(available_format); rating > max) {
            best_format = available_format;
            max = rating;
        }
    }

    std::cout << "Selected format: " << vk::to_string(best_format.format) << " "
              << vk::to_string(best_format.colorSpace) << std::endl;

    return best_format;
}

vk::PresentModeKHR
Application::choosePresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes,
                               const std::vector<vk::PresentModeKHR> &present_mode_preferences) {
    std::unordered_map<vk::PresentModeKHR, bool> present_mode_availability;

    for (auto &available_present_mode : available_present_modes) {
        present_mode_availability[available_present_mode] = true;
    }

    for (auto &present_mode : present_mode_preferences) {
        if (present_mode_availability[present_mode])
            return present_mode;
    }

    return vk::PresentModeKHR::eFifo; // Guaranteed to be available
}

void Application::run() {
    timer.restart();

    mainLoop();

    const float fps = static_cast<float>(frameCount) / timer.reset().asSeconds();
    std::cout << "Framerate: " << fps << " FPS" << std::endl;
}

void Application::initVulkan() {
    if (!sf::Vulkan::isAvailable()) {
        throw std::runtime_error("Vulkan is not available");
    }

    const auto layers = selectLayers();

    const auto instance_extensions = selectExtensions();

    createInstance(layers, instance_extensions);

#ifdef VALIDATION_LAYERS
    setupDebugMessenger();
#endif

    createSurface();

    const std::vector<std::string_view> device_extensions = {vk::KHRSwapchainExtensionName,
                                                             vk::EXTHdrMetadataExtensionName};
    selectPhysicalDevice(device_extensions);

    createLogicalDevice(layers, device_extensions);

    createSwapChain();
    createImageViews();

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

    vk::StructureChain instance_info_chain(instance_info
#ifdef VALIDATION_LAYERS
                                           ,
                                           debug_utils_messenger_create_info
#endif
    );

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

    required_extensions.emplace_back(vk::EXTSwapchainColorSpaceExtensionName);

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
    debugMessenger = instance.createDebugUtilsMessengerEXT(debug_utils_messenger_create_info);
}

void Application::mainLoop() {

    const auto on_resize = [this](const sf::Event::Resized &) { recreateSwapChain(); };

    const auto on_focus_lost = [this](const sf::Event::FocusLost &) {
        while (not window.waitEvent()->is<sf::Event::FocusGained>())
            ;
    };

    const auto on_close = [this](const sf::Event::Closed &) {
        device.waitIdle();
        window.close();
    };

    while (true) {
        window.handleEvents(on_resize, on_focus_lost, on_close);
        if (!window.isOpen())
            break;

        drawFrame();
    }
}

int Application::ratePhysicalDevice(
    const vk::raii::PhysicalDevice &physical_device,
    const std::vector<std::string_view> &requested_extensions) const {

    const auto properties = physical_device.getProperties();
    const auto features = physical_device.getFeatures();

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

    auto features2 =
        physical_device
            .getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features>();

    if (!(ApplicationQueueFamilies(physical_device, surface).isComplete() &&
          features.geometryShader && checkDeviceExtensions(physical_device, requested_extensions) &&
          ApplicationSwapChainDetails(physical_device, surface).isValid() &&
          features2.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters)) {
        return 0;
    }

    return score;
}

void Application::selectPhysicalDevice(const std::vector<std::string_view> &requested_extensions) {
    const std::vector<vk::raii::PhysicalDevice> physical_devices =
        instance.enumeratePhysicalDevices();
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

    queueFamilies = ApplicationQueueFamilies(physicalDevice, surface);
    swapChainDetails = ApplicationSwapChainDetails(physicalDevice, surface);

    std::cout << "Selected Device: " << physicalDevice.getProperties().deviceName << std::endl;
}

void Application::createLogicalDevice(const std::vector<std::string_view> &layers,
                                      const std::vector<std::string_view> &extensions) {
    std::vector<uint32_t> indices = queueFamilies.getQueueFamilyIndices();
    std::set unique_queue_indices(indices.begin(), indices.end());

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    for (uint32_t queue_family_index : unique_queue_indices) {
        auto queue_priority = {1.0f};
        vk::DeviceQueueCreateInfo queue_create_info({}, queue_family_index, queue_priority);
        queue_create_infos.push_back(queue_create_info);
    }

    constexpr auto enabled_features = [] {
        vk::PhysicalDeviceFeatures features;
        features.geometryShader = true;
#ifdef NDEBUG
        features.robustBufferAccess = false;
#endif

        return features;
    }();

    auto c_layers = to_c_strings(layers);
    auto c_extensions = to_c_strings(extensions);

    vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceVulkan11Features>
        device_create_info_chain{
            vk::DeviceCreateInfo({}, queue_create_infos, c_layers, c_extensions, &enabled_features),
            vk::PhysicalDeviceVulkan11Features().setShaderDrawParameters(true)};

    device = physicalDevice.createDevice(device_create_info_chain.get<vk::DeviceCreateInfo>());

    graphicsQueue = device.getQueue(indices[0], 0);
}

void Application::createSurface() {
    VkSurfaceKHR vk_surface;
    if (!window.createVulkanSurface(*instance, vk_surface))
        throw std::runtime_error("Failed to create Vulkan surface");
    surface = vk::raii::SurfaceKHR(instance, vk_surface);
}

vk::Extent2D Application::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const {
    const sf::Vector2u window_size = window.getSize();

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    const vk::Extent2D extent(std::clamp(window_size.x, capabilities.minImageExtent.width,
                                         capabilities.maxImageExtent.width),
                              std::clamp(window_size.y, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height));
    return extent;
}

void Application::createSwapChain() {
    const vk::SurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swapChainDetails.formats);
    const vk::PresentModeKHR present_mode =
        choosePresentMode(swapChainDetails.presentModes, {
                                                             vk::PresentModeKHR::eMailbox,
                                                             vk::PresentModeKHR::eFifoRelaxed,
                                                             vk::PresentModeKHR::eFifo,
                                                         });

    const vk::Extent2D extent = chooseSwapExtent(swapChainDetails.capabilities);

    uint32_t image_count = swapChainDetails.capabilities.minImageCount + 1;
    if (swapChainDetails.capabilities.maxImageCount > 0 &&
        image_count > swapChainDetails.capabilities.maxImageCount) {
        image_count = swapChainDetails.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swap_chain_create_info(
        {}, surface, image_count, surface_format.format, surface_format.colorSpace, extent, 1,
        vk::ImageUsageFlagBits::eColorAttachment);

    swap_chain_create_info.preTransform = swapChainDetails.capabilities.currentTransform;
    swap_chain_create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swap_chain_create_info.presentMode = present_mode;
    swap_chain_create_info.clipped = true;

    if (queueFamilies.areUnique()) {
        const auto queue_families = queueFamilies.getQueueFamilyIndices();
        swap_chain_create_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swap_chain_create_info.setQueueFamilyIndices(queue_families);
    } else {
        swap_chain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapChain = device.createSwapchainKHR(swap_chain_create_info);

    swapChainImageFormat = surface_format.format;
    swapChainExtent = extent;
    swapChainImages = swapChain.getImages();

    device.setHdrMetadataEXT(*swapChain,
                             vk::HdrMetadataEXT({0.708, 0.292}, {0.170, 0.797}, {0.131, 0.046},
                                                {0.3127, 0.3290}, 400, 0, 400, 400));
}

void Application::recreateSwapChain() {
    device.waitIdle();

    swapChainFramebuffers.clear();
    swapChainImageViews.clear();
    swapChain = nullptr;

    swapChainDetails = ApplicationSwapChainDetails(physicalDevice, surface);

    createSwapChain();
    createImageViews();
    createFramebuffers();
}

void Application::createImageViews() {
    swapChainImageViews.reserve(swapChainImages.size());

    constexpr vk::ImageSubresourceRange subresource_range(vk::ImageAspectFlagBits::eColor, 0, 1, 0,
                                                          1);

    for (const auto &image : swapChainImages) {
        vk::ImageViewCreateInfo create_info({}, image, vk::ImageViewType::e2D, swapChainImageFormat,
                                            {}, subresource_range);

        swapChainImageViews.emplace_back(device, create_info);
    }
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

    constexpr vk::SubpassDependency subpass_dependency(
        vk::SubpassExternal, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput, {},
        vk::AccessFlagBits::eColorAttachmentWrite);

    const vk::RenderPassCreateInfo render_pass_info({}, color_attachments, subpass,
                                                    subpass_dependency);

    renderPass = device.createRenderPass(render_pass_info);
}

void Application::createGraphicsPipeline() {

#include "triangle.spv.h"

    assert((void("Invalid SPIR-V magic number"), triangle[0] == 0x07230203));

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
    const vk::CommandPoolCreateInfo pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                              queueFamilies.graphicsFamily.value());

    commandPool = device.createCommandPool(pool_info);
}

void Application::createCommandBuffers() {
    commandBuffers.reserve(maxFramesInFlight);

    const vk::CommandBufferAllocateInfo command_buffer_allocate_info(
        commandPool, vk::CommandBufferLevel::ePrimary, maxFramesInFlight);

    commandBuffers = device.allocateCommandBuffers(command_buffer_allocate_info);
}

void Application::recordCommandBuffer(const vk::raii::CommandBuffer &command_buffer,
                                      const uint32_t image_index) const {
    command_buffer.begin(
        vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    constexpr vk::ClearValue clear_color_value(
        vk::ClearColorValue(std::array{0.0f, 0.0f, 0.0f, 1.0f}));
    const vk::RenderPassBeginInfo render_pass_info(renderPass, swapChainFramebuffers[image_index],
                                                   vk::Rect2D({}, swapChainExtent),
                                                   clear_color_value);

    command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    command_buffer.draw(3, 1, 0, 0);

    command_buffer.endRenderPass();

    command_buffer.end();
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

    const auto &current_command_buffer = commandBuffers[currentFrame];
    const auto &current_image_available_semaphore = imageAvailableSemaphores[currentFrame];
    const auto &current_render_finished_semaphore = renderFinishedSemaphores[currentFrame];
    const auto &current_in_flight_fence = inFlightFences[currentFrame];

    while (vk::Result::eTimeout ==
           device.waitForFences({current_in_flight_fence}, true, UINT64_MAX))
        ;

    auto [result, image_index] =
        swapChain.acquireNextImage(UINT64_MAX, current_image_available_semaphore);
    switch (result) {
    case vk::Result::eErrorOutOfDateKHR:
        recreateSwapChain();
        return;
    case vk::Result::eSuccess:
    case vk::Result::eSuboptimalKHR:
        break;
    default:
        throw std::runtime_error("Failed to acquire swap chain image: " + vk::to_string(result));
    }

    device.resetFences({current_in_flight_fence});

    current_command_buffer.reset();
    recordCommandBuffer(current_command_buffer, image_index);

    constexpr std::array<vk::PipelineStageFlags, 1> wait_stages{
        vk::PipelineStageFlagBits::eColorAttachmentOutput};

    const vk::SubmitInfo submit_info(*current_image_available_semaphore, wait_stages,
                                     *current_command_buffer, *current_render_finished_semaphore);

    graphicsQueue.submit(submit_info, current_in_flight_fence);

    const vk::PresentInfoKHR present_info(*current_render_finished_semaphore, *swapChain,
                                          image_index);
    result = graphicsQueue.presentKHR(present_info);
    switch (result) {
    case vk::Result::eErrorOutOfDateKHR:
    case vk::Result::eSuboptimalKHR:
        recreateSwapChain();
        break;
    case vk::Result::eSuccess:
        break;
    default:
        throw std::runtime_error("Failed to present swap chain image: " + vk::to_string(result));
    }

    currentFrame = ++frameCount % maxFramesInFlight;
}