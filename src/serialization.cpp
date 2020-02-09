#include <boost/property_tree/xml_parser.hpp>

#include "serialization.h"
#include "joymath.h"

namespace joytracer {
    namespace pt = boost::property_tree;

    /// Custom translator for vec3
    class Vec3Translator
    {
    public:
        typedef std::string           internal_type;
        typedef std::array<double, 3> external_type;

        /// Converts a string to vec3.
        /// @str: The comma separated representation of a vec3.
        /// \returns: a Vec3.
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
    };

    Scene load_scene(const std::string &filename) {
        pt::ptree pt;
        std::vector<std::unique_ptr<Surface>> surfaces;
        std::array<double, 3> sky_color;
        std::array<double, 3> sunlight_normal;

        read_xml(filename, pt);

        std::map<std::string, const std::function<void(const pt::ptree::value_type &)>> node_handlers {
            {"floor", [&surfaces](const pt::ptree::value_type &node){
                surfaces.push_back(std::make_unique<Floor>());
            }},
            {"sphere", [&surfaces](const pt::ptree::value_type &node){
                double radius = node.second.get("radius", 0.0);
                auto center = node.second.get("center", std::array{0.0, 0.0, 0.0}, Vec3Translator());
                auto color = node.second.get("color", std::array{0.0, 0.0, 0.0}, Vec3Translator());
                surfaces.push_back(std::make_unique<Sphere>(
                    radius, center, color
                ));
            }},
            {"sky-color", [&sky_color](const pt::ptree::value_type &node){
                sky_color = node.second.get_value(std::array{0.0, 0.0, 0.0}, Vec3Translator());
            }},
            {"sunlight-normal", [&sunlight_normal](const pt::ptree::value_type &node){
                sunlight_normal = normalize(node.second.get_value(std::array{0.0, 0.0, 0.0}, Vec3Translator()));
            }},
            {"triangle", [&surfaces](const pt::ptree::value_type &node){
                std::array<std::array<double, 3>, 3> vertices;
                std::array<double, 3> color;
                auto vert_iterator = vertices.begin();

                std::map<std::string, const std::function<void(const pt::ptree::value_type &)>> node_handlers {
                    {"vert", [&vert_iterator](const pt::ptree::value_type &node){
                        *(vert_iterator++) = node.second.get_value(std::array{0.0, 0.0, 0.0}, Vec3Translator());
                    }},
                    {"color", [&color](const pt::ptree::value_type &node){
                        color = node.second.get_value(std::array{0.0, 0.0, 0.0}, Vec3Translator());
                    }},
                };

                for (const auto &node: node.second) {
                    auto handler = node_handlers.find(node.first);
                    if (handler != node_handlers.end()) handler->second(node);
                }

                surfaces.push_back(std::make_unique<Triangle>(vertices, color));
            }},
        };

        for (const auto &node: pt.get_child("scene")) {
            auto handler = node_handlers.find(node.first);
            if (handler != node_handlers.end()) handler->second(node);
        }

        return Scene(std::move(surfaces), sky_color, sunlight_normal);
    }
} // namespace joytracer
