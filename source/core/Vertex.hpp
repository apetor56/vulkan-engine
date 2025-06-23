#pragma once

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

namespace ve {

struct Vertex {
    glm::vec3 position{};
    float uv_x{};
    glm::vec3 normal{};
    float uv_y{};
    glm::vec4 color{ 1.0F };
};

} // namespace ve
