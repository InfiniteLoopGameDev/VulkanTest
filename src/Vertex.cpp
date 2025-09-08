#include "Vertex.h"

vk::VertexInputBindingDescription Vertex::getBindingDescriptions() {
    return {0, static_cast<uint32_t>(sizeof(Vertex)), vk::VertexInputRate::eVertex};
}

std::array<vk::VertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions() {
    return {
        vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position)),
        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color))
    };
}

