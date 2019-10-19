#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include <SDL2/SDL.h>
#include "joytracer.h"

using namespace std::string_literals;

namespace joytracer {
    class SDL {
    public:
        SDL() {
            if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                throw std::runtime_error("SDL uninitialized"s);
            }
        }
        ~SDL() {
            SDL_Quit();
        }
    };

    class SDLSurface {
    private:
        SDL_Surface* m_surface;
    public:
        SDLSurface(SDL_Surface* surface) :
            m_surface(surface)
        {
        }
        SDLSurface(Uint32 flags,
            int    width,
            int    height,
            int    depth,
            Uint32 r_mask,
            Uint32 g_mask,
            Uint32 b_mask,
            Uint32 a_mask)
        {
            m_surface = SDL_CreateRGBSurface(flags, width, height, depth,
                                   r_mask, g_mask, b_mask, a_mask);
            if (m_surface == nullptr) {
                throw std::runtime_error("SDL_CreateRGBSurface failed! SDL_Error: "s + SDL_GetError());
            }
        }
        ~SDLSurface() {
            SDL_FreeSurface(m_surface);
        }

        void blit_to(SDLSurface &destination) {
            SDL_BlitSurface(m_surface, nullptr, destination.m_surface, nullptr);
        }

        // Dangerous! we should return a lock token!
        void lock() {
            if (SDL_LockSurface(m_surface)) {
                throw std::runtime_error("SDL_LockSurface failed! SDL_Error: "s + SDL_GetError());
            }
        }

        void unlock() {
            SDL_UnlockSurface(m_surface);
        }

        void set_pixel(int x, int y, uint32_t pixel) {
            uint32_t *target_pixel = reinterpret_cast<uint32_t*>(
                reinterpret_cast<uint8_t*>(m_surface->pixels) +
                y * m_surface->pitch + x * sizeof(uint32_t)
            );
            *target_pixel = pixel;
        }
    };

    class SDLWindow {
    private:
        SDL_Window* m_window;
    public:
        SDLWindow(const std::string &title, int width, int height) {
            m_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

            if (m_window == nullptr) {
                throw std::runtime_error("Window could not be created! SDL_Error: "s + SDL_GetError());
            }
        }
        ~SDLWindow() {
            SDL_DestroyWindow(m_window);
        }
        SDLSurface get_surface() {
            return SDLSurface(SDL_GetWindowSurface(m_window));
        }
        void update_surface() {
            SDL_UpdateWindowSurface(m_window);
        }
    };

    void quick_and_dirty_sdl_loop(std::function<void()> p) {
        while (true) {
            // Get the next event
            SDL_Event event;
            if (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    // Break out of the loop on quit
                    break;
                }

                p();
            }
        }
    }
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
        std::array<double, 3>{1.0, 0.5, 0.5}
    ));
    return v;
}

int main() {
    joytracer::SDL sdl;
    joytracer::SDLWindow sdl_window("Joytracer", 640, 480);
    joytracer::SDLSurface main_surface = sdl_window.get_surface();
    joytracer::SDLSurface backbuffer(0, 640, 480, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
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
    joytracer::quick_and_dirty_sdl_loop([&]() -> void {
        backbuffer.blit_to(main_surface);
        sdl_window.update_surface();
    });
    return 0;
}
