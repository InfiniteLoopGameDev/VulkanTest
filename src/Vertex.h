#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescriptions();

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
};