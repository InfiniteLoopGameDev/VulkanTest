//
// Created by coren on 10/03/2025.
//

#ifndef VULKANTEST_QUEUEFAMILYINDICIES_H
#define VULKANTEST_QUEUEFAMILYINDICIES_H

#include <optional>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

#endif //VULKANTEST_QUEUEFAMILYINDICIES_H
