#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace sim {
    struct alignas(16) Particle {
        glm::vec4 position_density;     // xyz = position, w = density
        glm::vec4 velocity_pressure;    // xyz = velocity, w = pressure
        glm::vec4 thermal_properties;   // x = temperature, y = heat_capacity, zw = reserved
    };

    static_assert(sizeof(Particle) == 48, "Particle struct must be exactly 48 bytes for std430 alignment");
}
