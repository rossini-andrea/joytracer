#pragma once
#include <functional>

#include <SDL2/SDL.h>

namespace sdl_wrapper {
    class SDL {
    public:
        SDL();
        ~SDL();
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
            Uint32 a_mask);
        ~SDLSurface();

        void blit_to(SDLSurface &destination);
        // Dangerous! we should return a lock token!
        void lock();
        void unlock();
        void set_pixel(int x, int y, uint32_t pixel);
    };

    class SDLWindow {
    private:
        SDL_Window* m_window;
    public:
        SDLWindow(const std::string &title, int width, int height);
        ~SDLWindow();
        SDLSurface get_surface() ;
        void update_surface();
    };

    void quick_and_dirty_sdl_loop(std::function<void()> p);
}
