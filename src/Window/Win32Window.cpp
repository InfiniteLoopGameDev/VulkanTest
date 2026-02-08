//
// Created by coren on 28/01/2026.
//

#include "Win32Window.h"

#include <winuser.h>

constexpr std::string_view class_name = "VulkanTest Window Class";

namespace window
{
    Win32Window::Win32Window(glm::ivec2 size, const std::string& title, bool fullscreen, bool resizable,
                             const std::function<void(events::event)>& event_handler) : Window(event_handler)
    {
        hinstance = GetModuleHandleA(nullptr);

        const WNDCLASSEXA wc{
            .cbSize = sizeof(wc),
            .lpfnWndProc = &Win32Window::winCallback,
            .cbWndExtra = sizeof(Win32Window*),
            .hInstance = hinstance,
            .lpszClassName = class_name.data(),
        };
        RegisterClassExA(&wc);

        const auto window_style = [fullscreen, resizable]
        {
            DWORD style = fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
            if (!resizable)
                style &= ~WS_THICKFRAME;
            return style;
        }();

        if (fullscreen)
        {
            size.x = GetSystemMetrics(SM_CXSCREEN);
            size.y = GetSystemMetrics(SM_CYSCREEN);
        }

        hwnd = CreateWindowExA(WS_EX_LEFT, class_name.data(), title.data(), window_style, CW_USEDEFAULT, CW_USEDEFAULT,
                               size.x, size.y, nullptr, nullptr, hinstance, this);

        ShowWindow(hwnd, SW_SHOWDEFAULT);
    }

    Win32Window::~Win32Window()
    {
        DestroyWindow(hwnd);
        UnregisterClassA(class_name.data(), hinstance);
    }

    bool Win32Window::isOpen()
    {
        return !IsIconic(hwnd);
    }

    bool Win32Window::isHdrSupported()
    {
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

        MONITORINFOEX mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(monitor, &mi);

        UINT32 num_path_array_elements = 0, num_mode_info_array_elements = 0;
        GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &num_path_array_elements, &num_mode_info_array_elements);

        std::vector<DISPLAYCONFIG_PATH_INFO> paths(num_path_array_elements);
        std::vector<DISPLAYCONFIG_MODE_INFO> modes(num_mode_info_array_elements);

        QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &num_path_array_elements, paths.data(),
                           &num_mode_info_array_elements, modes.data(), NULL);

        DISPLAYCONFIG_PATH_INFO display_path;

        for (const auto& path : paths)
        {
            DISPLAYCONFIG_SOURCE_DEVICE_NAME source_name = {};
            source_name.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
            source_name.header.size = sizeof(source_name);
            source_name.header.adapterId = path.sourceInfo.adapterId;
            source_name.header.id = path.sourceInfo.id;

            if (DisplayConfigGetDeviceInfo(&source_name.header) == ERROR_SUCCESS)
            {
                std::wstring_view gdi_name(source_name.viewGdiDeviceName);
                std::string_view sz_name(mi.szDevice);
                std::wstring wsz_name(sz_name.begin(), sz_name.end());
                if (gdi_name == wsz_name)
                    display_path = path;
            }
        }

        DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO device_info{
            .header = {
                .type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,
                .size = sizeof(device_info),
                .adapterId = display_path.sourceInfo.adapterId,
                .id = display_path.sourceInfo.id,
            }
        };
        DisplayConfigGetDeviceInfo(&device_info.header);

        return device_info.advancedColorEnabled;
    }

    glm::ivec2 Win32Window::getSize()
    {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        return {rect.right - rect.left, rect.bottom - rect.top};
    }

    void Win32Window::setSize(const glm::ivec2 size)
    {
        SetWindowPos(hwnd, nullptr, 0, 0, size.x, size.y, SWP_NOMOVE);
    }

    void Win32Window::pollEvents()
    {
        MSG msg = {};
        while (PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    HWND Win32Window::getHwnd()
    {
        return this->hwnd;
    }

    vk::raii::SurfaceKHR Win32Window::createVulkanSurface(vk::raii::Instance const& instance,
                                                          vk::Optional<const vk::AllocationCallbacks> allocator)
    {
        const vk::Win32SurfaceCreateInfoKHR create_info({}, this->hinstance, this->hwnd);
        return instance.createWin32SurfaceKHR(create_info, allocator);
    }

    constexpr std::vector<std::string_view> Win32Window::getVulkanRequiredInstanceExtensions()
    {
        return {vk::KHRSurfaceExtensionName, vk::KHRWin32SurfaceExtensionName};
    }

    LRESULT Win32Window::winCallback(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
    {
        const auto window = reinterpret_cast<Win32Window*>(GetWindowLongPtrA(hwnd, 0));

        switch (message)
        {
        case WM_NCCREATE:
            {
                const auto create = reinterpret_cast<CREATESTRUCT*>(l_param);
                SetWindowLongPtr(hwnd, 0, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
                break;
            }
        case WM_SIZE:
            {
                if (w_param == SIZE_MINIMIZED || w_param == SIZE_RESTORED || w_param == SIZE_MINIMIZED)
                    window->eventHandler(events::VisibilityChange(w_param != SIZE_MINIMIZED));
                else
                    window->eventHandler(events::Resize({
                        LOWORD(l_param), // Width
                        HIWORD(l_param) // Height
                    }));
                break;
            }
        case WM_CLOSE:
            window->eventHandler(events::Close{});
            return true;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            break;
        }
        return DefWindowProc(hwnd, message, w_param, l_param);;
    }
}
