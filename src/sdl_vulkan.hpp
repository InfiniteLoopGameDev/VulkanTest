#pragma once

#include <vulkan/vulkan.hpp>

#include <SDL2pp/Window.hh>

namespace SDL2pp {
    class Vulkan {
    public:
        explicit Vulkan(Window &window, const char *path);

        ~Vulkan();

        std::vector<std::string> GetInstanceExtensions() const;

        vk::SurfaceKHR CreateSurface(vk::Instance instance) const;

    private:
        Window *window;
    };
}
