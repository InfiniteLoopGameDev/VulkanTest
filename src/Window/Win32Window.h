//
// Created by coren on 28/01/2026.
//

#ifndef VULKANTEST_WIN32POINTER_H
#define VULKANTEST_WIN32POINTER_H

#include <functional>
#include <thread>
#include <future>

#include "Window.h"

namespace window
{
    class Win32Window : public Window
    {
    public:
        Win32Window(glm::ivec2 size, const std::string& title, bool fullscreen, bool resizable,
                    const std::function<void(events::event)>& event_handler);
        ~Win32Window() override;

        bool isOpen() override;
        bool isHdrSupported() override;
        glm::ivec2 getSize() override;
        void setSize(glm::ivec2 size) override;

        void pollEvents() override;

        vk::raii::SurfaceKHR createVulkanSurface(vk::raii::Instance const& instance,
                                                 vk::Optional<const vk::AllocationCallbacks> allocator) override;

        constexpr std::vector<std::string_view> getVulkanRequiredInstanceExtensions() override;

        HWND getHwnd() override;

    private:
        HWND hwnd;
        HINSTANCE hinstance;

        static LRESULT CALLBACK winCallback(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
    };
}


#endif //VULKANTEST_WIN32POINTER_H
