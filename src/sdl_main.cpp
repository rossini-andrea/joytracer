#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

#include "joymath.h"
#include "joytracer.h"
#include "sdl_wrapper.h"

auto make_pyramid() {
    std::vector<std::unique_ptr<joytracer::Surface>> v;

    // South face
    v.push_back(std::make_unique<joytracer::Triangle>(
        std::array<std::array<double, 3>, 3>({
            std::array<double, 3>{-1.0, 14.0, 0.0},
            std::array<double, 3>{7.0, 14.0, 0.0},
            std::array<double, 3>{3.0, 18.0, 5.0}
        }),
        std::array<double, 3>{0.8, 0.8, 0.4}
    ));

    // West face
    v.push_back(std::make_unique<joytracer::Triangle>(
        std::array<std::array<double, 3>, 3>({
            std::array<double, 3>{-1.0, 22.0, 0.0},
            std::array<double, 3>{-1.0, 14.0, 0.0},
            std::array<double, 3>{3.0, 18.0, 5.0}
        }),
        std::array<double, 3>{0.8, 0.8, 0.4}
    ));

    // North face
    v.push_back(std::make_unique<joytracer::Triangle>(
        std::array<std::array<double, 3>, 3>({
            std::array<double, 3>{7.0, 22.0, 0.0},
            std::array<double, 3>{-1.0, 22.0, 0.0},
            std::array<double, 3>{3.0, 18.0, 5.0}
        }),
        std::array<double, 3>{0.8, 0.8, 0.4}
    ));

    // East face
    v.push_back(std::make_unique<joytracer::Triangle>(
        std::array<std::array<double, 3>, 3>({
            std::array<double, 3>{7.0, 22.0, 0.0},
            std::array<double, 3>{7.0, 14.0, 0.0},
            std::array<double, 3>{3.0, 18.0, 5.0}
        }),
        std::array<double, 3>{0.8, 0.8, 0.4}
    ));

    return v;
}

auto make_scene_surfaces() {
    std::vector<std::unique_ptr<joytracer::Surface>> v;
    v.push_back(std::make_unique<joytracer::Floor>());
    v.push_back(std::make_unique<joytracer::Sphere>(
        0.5, std::array<double, 3>{1.0, 3.0, 0.5},
        std::array<double, 3>{0.0, 0.1, 1.0}
    ));
    v.push_back(std::make_unique<joytracer::Sphere>(
        0.5, std::array<double, 3>{-1.0, 3.5, 0.5},
        std::array<double, 3>{0.0, 0.8, 0.1}
    ));
    v.push_back(std::make_unique<joytracer::Sphere>(
        1.0, std::array<double, 3>{0.0, 6.0, 1.0},
        std::array<double, 3>{1.0, 1.0, 1.0}
    ));

    for (auto&& surface: make_pyramid()) {
        v.push_back(std::move(surface));
    }

    return v;
}

int main() {
    sdl_wrapper::SDL sdl;
    sdl_wrapper::SDLWindow sdl_window("Joytracer", 640, 480);
    sdl_wrapper::SDLSurface main_surface = sdl_window.get_surface();
    sdl_wrapper::SDLSurface backbuffer(0, 640, 480, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    joytracer::Scene test_scene(
        make_scene_surfaces(),
        {0.0, 0.40, 0.80}
    );
    joytracer::Camera fixed_camera{};
    fixed_camera.set_focal_distance(1.0);
    fixed_camera.set_plane_size(1.0, 480.0 / 640.0);
    fixed_camera.set_position({0.0, 0.0, 1.77});
    auto fixed_frame = fixed_camera.render_scene(test_scene, 640, 480);
    backbuffer.lock();

    for (int y = 0; y < 480; ++y) {
        for (int x = 0; x < 640; ++x) {
            int i = y * 640 + x;
            backbuffer.set_pixel(x, y, 0xff000000 |
                (static_cast<uint32_t>(255 * fixed_frame[i][0])) |
                (static_cast<uint32_t>(255 * fixed_frame[i][1])) << 8 |
                (static_cast<uint32_t>(255 * fixed_frame[i][2])) << 16);
        }
    }

    backbuffer.unlock();
    sdl_wrapper::quick_and_dirty_sdl_loop([&]() -> void {
        backbuffer.blit_to(main_surface);
        sdl_window.update_surface();
    },
    [&](int x, int y) -> void {
        double surface_y = (480.0 / 640.0) * (0.5 - static_cast<double>(y) / 480);
        double surface_x = (static_cast<double>(x) / 640 - 0.5);
        auto color = test_scene.trace_ray(joytracer::Ray(
                    {0.0, 0.0, 1.77},
                    joytracer::normalize(std::array<double, 3>{surface_x, 1.0, surface_y})
                ), 10);
        std::cout << "Color of clicked point: " << color[0] << ", " << color[1] << ", " << color[2] << ", " << '\n';
    }
    );
    return 0;
}
