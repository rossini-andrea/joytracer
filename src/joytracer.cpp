#include <algorithm>

#include "joymath.h"
#include "joytracer.h"

namespace joytracer {
    std::optional<HitPoint> project_ray_on_plane(
        const Ray &ray,
        const std::array<double, 3> &plane_origin,
        const std::array<double, 3> &plane_normal) {
        // Calculate if it may hit
        auto denom = dot(ray.get_normal(), plane_normal);

        if (denom > -epsilon && denom < epsilon) {
            return std::nullopt;
        }

        auto distance = dot(
            (plane_origin - ray.get_origin()),
            plane_normal) / denom;

        // Ignore if behind
        if (distance <= 0.0) {
            return std::nullopt;
        }

        return HitPoint(distance, ray.get_normal() * distance + ray.get_origin());
    }

    Triangle::Triangle(
            const std::array<std::array<double, 3>, 3> &vertices,
            const std::array<double, 3> &color
        ) : m_vertices(vertices), m_color(color),
            m_normal(normalize(cross(
                vertices[1] - vertices[0],
                vertices[2] - vertices[1]
            ))) {
    }

    std::optional<HitResult> Triangle::hit_test(const Ray &ray) const {
        auto projection = project_ray_on_plane(ray, *m_vertices.begin(), m_normal);

        if (!projection) {
            return std::nullopt;
        }

        auto hit_point = projection->point();

        // Check if the point is "behind" all three edges at once. If so
        // it is between the edges.
        if (dot(m_normal, cross(m_vertices[1] - m_vertices[0], hit_point - m_vertices[1])) > 0 &&
            dot(m_normal, cross(m_vertices[2] - m_vertices[1], hit_point - m_vertices[2])) > 0 &&
            dot(m_normal, cross(m_vertices[0] - m_vertices[2], hit_point - m_vertices[0])) > 0) {
            return HitResult(projection->distance(), hit_point, m_color);
        }

        return std::nullopt;
    }

    std::optional<HitResult> Floor::hit_test(const Ray &ray) const {
        auto projection = project_ray_on_plane(ray, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0});

        if (!projection) {
            return std::nullopt;
        }

        auto hit_point = projection->point();
        long is_x_odd = static_cast<long>(floorf(hit_point[0])) & 1;
        long is_y_odd = static_cast<long>(floorf(hit_point[1])) & 1;
        return HitResult(projection->distance(), hit_point,
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
