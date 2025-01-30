#include "sdl_vulkan.hpp"

#include <SDL2/SDL_vulkan.h>

#include <SDL2pp/Exception.hh>

namespace SDL2pp {
    Vulkan::Vulkan(Window &window, const char *path) {
        SDL_Vulkan_LoadLibrary(path);
        this->window = &window;
    }

    Vulkan::~Vulkan() {
        SDL_Vulkan_UnloadLibrary();
    }

    std::vector<std::string> Vulkan::GetInstanceExtensions() const {
        unsigned int count;
        if (SDL_Vulkan_GetInstanceExtensions(window->Get(), &count, nullptr) != SDL_TRUE) {
            throw SDL2pp::Exception("SDL_Vulkan_GetInstanceExtensions");
        }

        std::vector<const char *> names(count);
        if (SDL_Vulkan_GetInstanceExtensions(window->Get(), &count, names.data()) != SDL_TRUE) {
            throw SDL2pp::Exception("SDL_Vulkan_GetInstanceExtensions");
        }

        std::vector<std::string> extensions;
        extensions.reserve(count);
        for (const char *name: names) {
            extensions.emplace_back(name);
        }

        return extensions;
    }

    vk::SurfaceKHR Vulkan::CreateSurface(vk::Instance instance) const {
        VkSurfaceKHR surface;
        if (SDL_Vulkan_CreateSurface(window->Get(), instance, &surface) != SDL_TRUE) {
            throw SDL2pp::Exception("SDL_Vulkan_CreateSurface");
        }
        return {surface};
    }
}

