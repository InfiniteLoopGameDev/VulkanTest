//
// Created by coren on 27/01/2026.
//

#ifndef VULKANTEST_WINDOWPOINTER_H
#define VULKANTEST_WINDOWPOINTER_H

#include <glm/vec2.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <functional>

#ifdef WIN32
#include <windef.h>
#endif

#include "../Events/Event.h"

namespace window
{
    class Window
    {
    public:
        virtual ~Window() = default;

        virtual bool isOpen() = 0;
        virtual bool isHdrSupported() = 0;
        virtual glm::ivec2 getSize() = 0;
        virtual void setSize(glm::ivec2 size) = 0;

        virtual void pollEvents() = 0;

        virtual vk::raii::SurfaceKHR createVulkanSurface(vk::raii::Instance const& instance)
        {
            return createVulkanSurface(instance, nullptr);
        };

        virtual vk::raii::SurfaceKHR createVulkanSurface(vk::raii::Instance const& instance,
                                                         vk::Optional<const vk::AllocationCallbacks> allocator) =
        0;

        virtual constexpr std::vector<std::string_view> getVulkanRequiredInstanceExtensions() = 0;

#ifdef WIN32
        virtual HWND getHwnd() = 0;
#endif

        explicit Window(const std::function<void(events::event)>& event_handler) : eventHandler(event_handler)
        {
        }

    protected:
        std::function<void(events::event)> eventHandler = nullptr;
    };
} // window_pointers

#endif //VULKANTEST_WINDOWPOINTER_H
