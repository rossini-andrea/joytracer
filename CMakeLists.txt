cmake_minimum_required(VERSION 3.0)
project(joytracer)
set(CMAKE_CXX_STANDARD 17)
add_executable(joytracer \
    src/joytracer.cpp \
    src/sdl_main.cpp \
    src/sdl_wrapper.cpp)
target_link_libraries (joytracer SDL2)
