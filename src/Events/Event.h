//
// Created by coren on 28/01/2026.
//

#ifndef VULKANTEST_EVENT_H
#define VULKANTEST_EVENT_H

#include <variant>

#include "Resize.h"
#include "VisibilityChange.h"

namespace events
{
    struct Close
    {
    };

    using event = std::variant<
        Resize,
        VisibilityChange,
        Close
    >;
}

#endif //VULKANTEST_EVENT_H
