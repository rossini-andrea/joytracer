#include <stdexcept>
#include <string>

#include "sdl_wrapper.h"

namespace sdl_wrapper {
    using namespace std::string_literals;

    SDL::SDL() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("SDL uninitialized"s);
        }
    }

    SDL::~SDL() {
        SDL_Quit();
    }

    SDLSurface::SDLSurface(Uint32 flags,
        int    width,
        int    height,
        int    depth,
        Uint32 r_mask,
        Uint32 g_mask,
        Uint32 b_mask,
        Uint32 a_mask)
    {
        m_surface.reset(SDL_CreateRGBSurface(flags, width, height, depth,
                                r_mask, g_mask, b_mask, a_mask));

        if (m_surface == nullptr) {
            throw std::runtime_error("SDL_CreateRGBSurface failed! SDL_Error: "s + SDL_GetError());
        }
    }

    SDLSurface::~SDLSurface() {
        SDL_FreeSurface(m_surface.release());
    }

    void SDLSurface::blit_to(const SDLSurface &destination) {
        SDL_BlitSurface(m_surface.get(), nullptr, destination.m_surface.get(), nullptr);
    }

    // Dangerous! we should return a lock token!
    void SDLSurface::lock() {
        if (SDL_LockSurface(m_surface.get()) != 0) {
            throw std::runtime_error("SDL_LockSurface failed! SDL_Error: "s + SDL_GetError());
        }
    }

    void SDLSurface::unlock() {
        SDL_UnlockSurface(m_surface.get());
    }

    void SDLSurface::set_pixel(int x, int y, uint32_t pixel) {
        auto *target_pixel = reinterpret_cast<uint32_t*>(
            reinterpret_cast<uint8_t*>(m_surface->pixels) +
            y * m_surface->pitch + x * sizeof(uint32_t)
        );
        *target_pixel = pixel;
    }

    SDLWindow::SDLWindow(const std::string &title, int width, int height) {
        m_window.reset(SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN));

        if (m_window == nullptr) {
            throw std::runtime_error("Window could not be created! SDL_Error: "s + SDL_GetError());
        }
    }

    void SDLWindow::Deleter::operator()(SDL_Window *w) {
        if (w == nullptr) {
            return;
        }

        SDL_DestroyWindow(w);
    }

    SDLSurface SDLWindow::get_surface() {
        return SDLSurface(SDL_GetWindowSurface(m_window.get()));
    }

    void SDLWindow::update_surface() {
        SDL_UpdateWindowSurface(m_window.get());
    }

    void quick_and_dirty_sdl_loop(
        const std::function<void()> repaint,
        const std::function<void(int x, int y)> onclick
    ) {
        while (true) {
            // Get the next event
            SDL_Event event;

            if (SDL_PollEvent(&event) != 0) {
                if (event.type == SDL_QUIT) {
                    // Break out of the loop on quit
                    break;
                }
                if (event.type == SDL_MOUSEBUTTONUP) {
                    onclick(event.button.x, event.button.y);
                }

                repaint();
            }
        }
    }
} // namespace sdl_wrapper
