cmake_minimum_required(VERSION 3.6)
project(joytracer)
set(CMAKE_CXX_STANDARD 17)

find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
)
if(NOT CLANG_TIDY_EXE)
    message(STATUS "clang-tidy not found.")
else()
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*")
endif()

find_package(Boost 1.72.0)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(joytracer
        "src/joytracer.cpp"
        "src/sdl_main.cpp"
        "src/sdl_wrapper.cpp")
    target_link_libraries(joytracer SDL2)
endif()

if(CLANG_TIDY_EXE)
    set_target_properties(joytracer
        PROPERTIES
        CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
endif()
