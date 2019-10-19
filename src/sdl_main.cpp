#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "sdl_wrapper.h"
#include "joytracer.h"

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
        std::array<double, 3>{1.0, 0.5, 0.5}
    ));
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
    joytracer::Camera fixed_camera;
    fixed_camera.set_focal_distance(1.0);
    fixed_camera.set_plane_size(640.0 / 480.0, 1.0);
    fixed_camera.set_position({0.0, 0.0, 1.77});
    auto fixed_frame = fixed_camera.render_scene(test_scene, 640, 480);
    backbuffer.lock();
    for (int y = 0; y < 480; ++y) {
        for (int x = 0; x < 640; ++x) {
            int i = y * 640 + x;
            backbuffer.set_pixel(x, y, 0xff000000 |
                ((uint32_t)(255 * fixed_frame[i][0])) |
                ((uint32_t)(255 * fixed_frame[i][1])) << 8 |
                ((uint32_t)(255 * fixed_frame[i][2])) << 16);
        }
    }
    backbuffer.unlock();
    sdl_wrapper::quick_and_dirty_sdl_loop([&]() -> void {
        backbuffer.blit_to(main_surface);
        sdl_window.update_surface();
    });
    return 0;
}
