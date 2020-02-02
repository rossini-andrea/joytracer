#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <mutex>
#include <boost/property_tree/xml_parser.hpp>

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
            std::array<double, 3>{7.0, 14.0, 0.0},
            std::array<double, 3>{7.0, 22.0, 0.0},
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

// Custom translator for vec3
class Vec3Translator
{
public:
    typedef std::string           internal_type;
    typedef std::array<double, 3> external_type;

    // Converts a string to vec3
    boost::optional<external_type> get_value(const internal_type& str)
    {
        std::stringstream ss(str);
        std::array<double, 3> result;

        for (double &d: result) {
            if (!ss.good()) {
                return boost::optional<external_type>(boost::none);
            }

            std::string substr;
            std::getline(ss, substr, ',');
            d = std::stod(substr);
        }

        return boost::optional<external_type>(result);
    }
/*
    // Converts a Vec3 to string
    boost::optional<internal_type> put_value(const external_type& vec)
    {
        std::stringstream ss("");

        for (const double &d: vec) {
            ss << d << ", ";
        }

        return boost::optional<internal_type>(ss);
    }*/
};

joytracer::Scene load_scene(const std::string &filename) {
    boost::property_tree::ptree pt;
    std::vector<std::unique_ptr<joytracer::Surface>> surfaces;
    std::array<double, 3> sky_color;
    std::array<double, 3> sunlight_normal;

    read_xml(filename, pt);

    std::map<std::string, const std::function<void(const boost::property_tree::ptree::value_type &)>> node_handlers {
        {"floor", [&surfaces](const boost::property_tree::ptree::value_type &node){
            surfaces.push_back(std::make_unique<joytracer::Floor>());
        }},
        {"sphere", [&surfaces](const boost::property_tree::ptree::value_type &node){
            double radius = node.second.get("radius", 0.0);
            auto center = node.second.get("center", std::array{0.0, 0.0, 0.0}, Vec3Translator());
            auto color = node.second.get("color", std::array{0.0, 0.0, 0.0}, Vec3Translator());
            surfaces.push_back(std::make_unique<joytracer::Sphere>(
                radius, center, color
            ));
        }},
        {"sky-color", [&sky_color](const boost::property_tree::ptree::value_type &node){
            sky_color = node.second.get_value(std::array{0.0, 0.0, 0.0}, Vec3Translator());
        }},
        {"sunlight-normal", [&sunlight_normal](const boost::property_tree::ptree::value_type &node){
            sunlight_normal = node.second.get_value(std::array{0.0, 0.0, 0.0}, Vec3Translator());
        }},
        {"triangle", [&surfaces](const boost::property_tree::ptree::value_type &node){
            std::array<std::array<double, 3>, 3> vertices;
            std::array<double, 3> color;
            auto vert_iterator = vertices.begin();

            std::map<std::string, const std::function<void(const boost::property_tree::ptree::value_type &)>> node_handlers {
                {"vert", [&vert_iterator](const boost::property_tree::ptree::value_type &node){
                    *(vert_iterator++) = node.second.get_value(std::array{0.0, 0.0, 0.0}, Vec3Translator());
                }},
                {"color", [&color](const boost::property_tree::ptree::value_type &node){
                    color = node.second.get_value(std::array{0.0, 0.0, 0.0}, Vec3Translator());
                }},
            };

            for (const auto &node: node.second) {
                auto handler = node_handlers.find(node.first);
                if (handler != node_handlers.end()) handler->second(node);
            }

            surfaces.push_back(std::make_unique<joytracer::Triangle>(vertices, color));
        }},
    };

    for (const auto &node: pt.get_child("scene")) {
        auto handler = node_handlers.find(node.first);
        if (handler != node_handlers.end()) handler->second(node);
    }

    return joytracer::Scene(surfaces, sky_color, sunlight_normal);
}

int main(int argc, char** argv) {
    const int screen_width = 640;
    const int screen_height = 480;

    if (argc != 2) {
        std::cerr << "No input scene specified!\n";
        return 1;
    }

    joytracer::Scene test_scene = load_scene(argv[1]);
    sdl_wrapper::SDL sdl;
    sdl_wrapper::SDLWindow sdl_window("Joytracer", screen_width, screen_height);
    sdl_wrapper::SDLSurface main_surface = sdl_window.get_surface();
    sdl_wrapper::SDLSurface backbuffer(0, screen_width, screen_height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    joytracer::Camera fixed_camera{};
    fixed_camera.set_focal_distance(1.0);
    fixed_camera.set_plane_size(1.0, static_cast<double>(screen_height) / static_cast<double>(screen_width));
    fixed_camera.set_position({0.0, 0.0, 1.77});
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
