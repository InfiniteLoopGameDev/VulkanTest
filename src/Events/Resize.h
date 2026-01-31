//
// Created by coren on 29/01/2026.
//

#ifndef VULKANTEST_RESIZE_H
#define VULKANTEST_RESIZE_H

#include <glm/vec2.hpp>

namespace events
{
    struct Resize
    {
        glm::ivec2 size;
    };
} // events

#endif //VULKANTEST_RESIZE_H
