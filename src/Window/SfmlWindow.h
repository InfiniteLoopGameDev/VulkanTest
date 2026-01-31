//
// Created by coren on 28/01/2026.
//

#ifndef VULKANTEST_SFMLPOINTER_H
#define VULKANTEST_SFMLPOINTER_H

#include <SFML/Window.hpp>

#include "Window.h"

namespace window
{
    class SfmlWindow : Window
    {
    public:
        SfmlWindow(sf::Window& window);

        vk::raii::SurfaceKHR createVulkanSurface(vk::raii::Instance const& instance,
                                              vk::Optional<const vk::AllocationCallbacks> allocator) override;

#ifdef WIN32
        HWND getHwnd() override;
#endif

    private:
        sf::Window& window;
    };
} // window_pointers

#endif //VULKANTEST_SFMLPOINTER_H
