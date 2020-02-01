#pragma once
#include <memory>
#include <functional>

#include <SDL2/SDL.h>

namespace sdl_wrapper {
    class SDL {
    private:
        SDL(const SDL&);
        SDL& operator=(const SDL&);
    public:
        SDL();
        ~SDL();
    };

    class SDLSurface {
    private:
        std::unique_ptr<SDL_Surface> m_surface;

        SDLSurface(const SDLSurface&);
        SDLSurface& operator=(const SDLSurface&);
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
            Uint32 a_mask);
        ~SDLSurface();

        void blit_to(const SDLSurface &destination);
        // Dangerous! we should return a lock token!
        void lock();
        void unlock();
        bool try_lock();
        void set_pixel(int x, int y, uint32_t pixel);
    };

    class SDLWindow {
    private:
        class Deleter {
        public:
            typedef SDL_Window *pointer;
            void operator()(SDL_Window *w);
        };

        std::unique_ptr<SDL_Window, Deleter> m_window;

        SDLWindow(const SDLWindow&);
        SDLWindow& operator=(const SDLWindow&);
    public:
        SDLWindow(const std::string &title, int width, int height);
        ~SDLWindow() = default;

        SDLWindow(SDLWindow&&) = default;
        SDLWindow& operator=(SDLWindow&&) = default;

        SDLSurface get_surface();
        void update_surface();
    };

    void quick_and_dirty_sdl_loop(const std::function<void()> &repaint,
        const std::function<void(int x, int y)> &onclick);
}
