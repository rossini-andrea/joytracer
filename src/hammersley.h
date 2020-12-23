#pragma once
#include <cmath>
#include <cstdint>
#include <memory>
#include <array>

/*
* Thanks to http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
*/
namespace hammersley {
    const double pi = std::acos(-1);

    constexpr double radicalinverse_vdc(uint32_t bits) {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return static_cast<double>(bits) * 2.3283064365386963e-10; // / 0x100000000
    }

    constexpr std::array<double, 2> hammersley2d(uint32_t i, uint32_t n) {
        return {static_cast<double>(i)/static_cast<double>(n), radicalinverse_vdc(i)};
    }

    std::array<double, 3> hemispheresample_uniform(double u, double v) {
        double phi = v * 2.0 * pi;
        double cosTheta = 1.0 - u;
        double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
        return {std::cos(phi) * sinTheta, std::sin(phi) * sinTheta, cosTheta};
    }
}
