#include <algorithm>

#include "joymath.h"
#include "joytracer.h"

namespace joytracer {
/*
    std::optional<HitResult> FlatSurface::hit_test(const Ray &ray) const {
        auto denom = dot(ray.normal(), m_normal);

        if (denom > -epsilon && denom < epsilon) {
            return std::nullopt;
        }

        auto numerator = dot((m_origin - ray.origin()), m_normal);
        auto hit_point = denom * ray.normal() + ray.origin();

        // TODO: check if hit_point is in the rectangle

        return std::nullopt;
    }*/
    std::optional<HitResult> Floor::hit_test(const Ray &ray) const {
        // Calculate if hit
        auto denom = dot(ray.get_normal(), {0.0, 0.0, 1.0});

        if (denom > -epsilon && denom < epsilon) {
            return std::nullopt;
        }

        auto distance = dot(
            (std::array<double, 3>{0.0, 0.0, 0.0} - ray.get_origin()),
            {0.0, 0.0, 1.0}) / denom;

        if (distance <= 0.0) {
            return std::nullopt;
        }

        auto hit_point = ray.get_normal() * distance + ray.get_origin();
        long is_x_odd = static_cast<long>(floorf(hit_point[0])) & 1;
        long is_y_odd = static_cast<long>(floorf(hit_point[1])) & 1;
        return HitResult(distance, hit_point,
            (is_x_odd == is_y_odd) ?
            std::array<double, 3>{1.0, 1.0, 1.0} :
            std::array<double, 3>{0.0, 0.0, 0.0});
    }

    std::optional<HitResult> Sphere::hit_test(const Ray &ray) const {
        auto origin_to_center = ray.get_origin() - m_center;
        auto origin_to_center_length = vector_length(origin_to_center);
        auto projection = dot(ray.get_normal(), origin_to_center);
        auto square = projection * projection -
        origin_to_center_length * origin_to_center_length +
        m_radius * m_radius;

        if (square < 0.0) {
            return std::nullopt;
        }

        double distance = square <= epsilon ?
            - projection :
            - std::sqrt(square) - projection;

        return HitResult(
            distance,
            ray.get_origin() + ray.get_normal() * distance,
            m_color);
    }

    std::array<double, 3> Scene::trace_ray(const Ray &ray) const {
        std::vector<std::optional<HitResult>> hits;
        std::transform(m_surfaces.begin(), m_surfaces.end(),
            std::back_inserter(hits),
            [=] (auto &s) -> auto { return s->hit_test(ray); });
        auto hits_end = std::remove_if(hits.begin(), hits.end(), [](auto &h) -> bool { return !h.has_value(); } );
        auto found = std::min_element(hits.begin(), hits_end,
            [] (auto &a, auto &b) -> bool {
                return (a->distance() < b->distance());
            });
        return found != hits_end ? found->value().color() : m_sky_color;
    }

    void Camera::set_orientation(const std::array<double, 3> &orientation) {
        // KISS: Write a first version that can render looking down the Y axis!
        /*double length = std::cos(orientation[0]);
        m_lookat = normalize<double, 3>({
            length * std::sin(orientation[1]),
            length * std::cos(orientation[1]),
            std::sin(orientation[0])
        });
        m_camera_up =
        m_orientation = orientation;*/
    }

    std::vector<std::array<double, 3>> Camera::render_scene(const Scene &scene, int width, int height) {
        std::vector<std::array<double, 3>> frame(width * height);

        for (int y = 0; y < height; ++y) {
            double surface_y = m_plane_height * (0.5 - static_cast<double>(y) / height);

            for (int x = 0; x < width; ++x) {
                double surface_x = m_plane_width * (static_cast<double>(x) / width - 0.5);
                frame[y * width + x] = scene.trace_ray(Ray(
                    m_position,
                    normalize(std::array<double, 3>{surface_x, m_focal_distance, surface_y})
                ));
            }
        }

        return frame;
    }
} // namespace joytracer
