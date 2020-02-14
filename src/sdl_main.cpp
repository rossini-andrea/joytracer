#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <mutex>

#include "joymath.h"
#include "joytracer.h"
#include "sdl_wrapper.h"
#include "serialization.h"

int main(int argc, char** argv) {
    const int screen_width = 640;
    const int screen_height = 480;

    if (argc != 2) {
        std::cerr << "No input scene specified!\n";
        return 1;
    }

    joytracer::Scene test_scene = joytracer::load_scene(argv[1]);
    sdl_wrapper::SDL sdl;
    sdl_wrapper::SDLWindow sdl_window("Joytracer", screen_width, screen_height);
    sdl_wrapper::SDLSurface main_surface = sdl_window.get_surface();
    sdl_wrapper::SDLSurface backbuffer(0, screen_width, screen_height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    joytracer::Camera fixed_camera{};
    fixed_camera.set_focal_distance(1.0);
    fixed_camera.set_plane_size(1.0, static_cast<double>(screen_height) / static_cast<double>(screen_width));
    fixed_camera.set_position(joytracer::Vec3({0.0, 0.0, 1.77}));
    fixed_camera.set_orientation({0.0, std::acos(-1) * 0.50, 0.0});
    auto fixed_frame = fixed_camera.render_scene(test_scene, screen_width, screen_height);

    // Scoped lock on the SDL surface.
    {
        std::scoped_lock backbuffer_lock(backbuffer);

        for (int y = 0; y < screen_height; ++y) {
            for (int x = 0; x < screen_width; ++x) {
                int i = y * screen_width + x;
                backbuffer.set_pixel(x, y, 0xff000000 |
                    (static_cast<uint32_t>(255 * fixed_frame[i][0])) |
                    (static_cast<uint32_t>(255 * fixed_frame[i][1])) << 8 |
                    (static_cast<uint32_t>(255 * fixed_frame[i][2])) << 16);
            }
        }
    }

    sdl_wrapper::quick_and_dirty_sdl_loop(
        // repaint
        [&]() -> void {
            backbuffer.blit_to(main_surface);
            sdl_window.update_surface();
        },
        // onclick
        [&](int x, int y) -> void {
            auto color = fixed_camera.test_point(test_scene, screen_width, screen_height, x, y);
            std::cout
                << "Color of (" << x << "," << y << "): "
                << color[0] << ", " << color[1] << ", " << color[2] << ", " << '\n';
        }
    );
    return 0;
}
