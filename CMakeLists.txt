cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
project(joytracer)

IF(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast /EHsc")
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")
ENDIF(MSVC)

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

if(DEFINED ENV{VCPKG_ROOT})
    if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
    endif()

    message("[INFO] VCPKG CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

    find_package(Boost REQUIRED COMPONENTS serialization)
else()
    message("[WARN] VCPKG_ROOT missing")

    find_package(Boost REQUIRED)
endif()

find_package(SDL2 REQUIRED)

add_executable(joytracer
    "src/joytracer.cpp"
    "src/sdl_main.cpp"
    "src/sdl_wrapper.cpp"
    "src/serialization.cpp")

target_include_directories(joytracer PRIVATE ${Boost_INCLUDE_DIRS})

if(MINGW)
    message("[INFO] Setting MinGW options.")
    target_link_libraries(joytracer PRIVATE mingw32 SDL2::SDL2main SDL2::SDL2 ${Boost_LIBRARIES})
else()
    target_link_libraries(joytracer PRIVATE SDL2::SDL2main SDL2::SDL2 ${Boost_LIBRARIES})
endif(MINGW)

if(CLANG_TIDY_EXE)
    set_target_properties(joytracer
        PROPERTIES
        CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
endif()
