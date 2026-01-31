//
// Created by coren on 28/01/2026.
//

#include "SfmlWindow.h"

namespace window
{
    SfmlWindow::SfmlWindow(sf::Window& window) : window(window)
    {
    }

    vk::raii::SurfaceKHR SfmlWindow::createVulkanSurface(vk::raii::Instance const& instance,
                                                      vk::Optional<const vk::AllocationCallbacks> allocator)
    {
        VkSurfaceKHR surface;

        if (!window.createVulkanSurface(*instance, surface, *allocator))
            throw std::runtime_error("Failed to create Vulkan surface");

        return vk::raii::SurfaceKHR(instance, surface, allocator);
    }

#ifdef WIN32
    HWND SfmlWindow::getHwnd()
    {
        return window.getNativeHandle();
    }
#endif
} // window_pointers
