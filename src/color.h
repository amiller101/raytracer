// Helpers for vectors representing a color.

#pragma once
#include "interval.h"
#include "vec3.h"

using color = Vec3;

//Human perception of brightness follows a logorithmic scale of RGB values
//So we gamut correct by squaring every color component.
inline double linear_to_gamma(double linear_component)
{
    if (linear_component > 0)
    {
        return std::sqrt(linear_component);
    }
    return 0;
}

// Write a color to output in PPM format
inline void write_color(std::ostream &out, const color& pixel_color) {
    auto r = pixel_color.r;
    auto g = pixel_color.g;
    auto b = pixel_color.b;

    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    //Map [0, 1) to [0, 255).
    static const interval intensity(0.000, 0.999);
    int rbyte = static_cast<int>(255.999 * intensity.clamp(r));
    int gbyte = static_cast<int>(255.999 * intensity.clamp(g));
    int bbyte = static_cast<int>(255.999 * intensity.clamp(b));

    std::cout << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}