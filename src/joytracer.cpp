#include <algorithm>
#include <iterator>
#include <random>

#include "hammersley.h"
#include "joytracer.h"

namespace joytracer {
    Color Color::blend(
        const std::vector<Color> &colors
    ) {
        return from_rgb(std::accumulate(
            colors.begin(), colors.end(),
            std::array{0.0, 0.0, 0.0},
            [&](const auto &accum, const auto &c) constexpr -> auto {
                return accum + c.m_value;
            }
        ) / static_cast<double>(colors.size()));
    }

    std::optional<HitPoint> project_ray_on_plane_frontface(
        const Ray &ray,
        const Vec3 &plane_origin,
        const Normal3 &plane_normal) {
        // Calculate if it may hit
        auto denom = dot(ray.get_normal(), plane_normal);

        if (denom > -epsilon) {
            return std::nullopt;
        }

        auto distance = dot(
            (plane_origin - ray.get_origin()),
            plane_normal) / denom;

        // Ignore if behind
        if (distance <= epsilon) {
            return std::nullopt;
        }

        return HitPoint(distance, ray.get_normal() * distance + ray.get_origin());
    }

    Triangle::Triangle(
            const std::array<Vec3, 3> &vertices,
            const Color &color
        ) : m_vertices(vertices), m_color(color),
            m_normal(cross(
                vertices[1] - vertices[0],
                vertices[2] - vertices[1]
            )) {
    }

    std::optional<HitResult> Triangle::hit_test(const Ray &ray) const {
        auto projection = project_ray_on_plane_frontface(ray, *m_vertices.begin(), m_normal);

        if (!projection) {
            return std::nullopt;
        }

        auto hit_point = projection->point();

        // Check if the point is "behind" all three edges at once. If so
        // it is between the edges.
        if (dot(m_normal, cross(m_vertices[1] - m_vertices[0], hit_point - m_vertices[1])) > 0 &&
            dot(m_normal, cross(m_vertices[2] - m_vertices[1], hit_point - m_vertices[2])) > 0 &&
            dot(m_normal, cross(m_vertices[0] - m_vertices[2], hit_point - m_vertices[0])) > 0) {
            return HitResult(projection->distance(), hit_point, m_normal, m_color);
        }

        return std::nullopt;
    }

    std::optional<HitResult> Floor::hit_test(const Ray &ray) const {
        auto projection = project_ray_on_plane_frontface(ray, Vec3({0.0, 0.0, 0.0}), Normal3(Vec3({0.0, 0.0, 1.0})));

        if (!projection) {
            return std::nullopt;
        }

        auto hit_point = projection->point();
        long is_x_odd = static_cast<long>(floorf(hit_point[0])) & 1;
        long is_y_odd = static_cast<long>(floorf(hit_point[1])) & 1;
        return HitResult(projection->distance(), hit_point,
            Normal3(std::array{0.0, 0.0, 1.0}),
            (is_x_odd == is_y_odd) ?
            Color::white() :
            Color::black());
    }

    std::optional<HitResult> Sphere::hit_test(const Ray &ray) const {
        auto origin_to_center = ray.get_origin() - m_center;
        auto origin_to_center_length = origin_to_center.vector_length();
        auto projection = dot(ray.get_normal(), origin_to_center);
        auto square = projection * projection -
        origin_to_center_length * origin_to_center_length +
        m_radius * m_radius;

        if (square < 0.0) {
            return std::nullopt;
        }

        double distance = square <= epsilon ?
            -projection :
            -projection - std::sqrt(square);

        // Ignore if behind
        if (distance <= epsilon) {
            return std::nullopt;
        }

        auto hit_point = ray.get_origin() + ray.get_normal() * distance;

        return HitResult(
            distance,
            hit_point,
            Normal3(hit_point - m_center),
            m_color);
    }

    std::optional<HitResult> Scene::trace_single_ray(const Ray &ray) const {
        std::vector<std::optional<HitResult>> hits;
        std::transform(m_surfaces.begin(), m_surfaces.end(),
            std::back_inserter(hits),
            [&] (auto &s) -> auto {
                return std::visit(HitTestVisitor(ray), s);
            });
        auto hits_end = std::remove_if(hits.begin(), hits.end(), [](auto &h) -> bool { return !h.has_value(); } );
        auto nearest_hit = std::min_element(hits.begin(), hits_end,
            [] (auto &a, auto &b) -> bool {
                return (a->distance() < b->distance());
            });

        if (nearest_hit == hits_end) {
            return std::nullopt;
        }

        return *nearest_hit;
    }

    Color Scene::trace_and_bounce_ray(const Ray &ray, int reflect) const {
        if (reflect == 0) {
            return Color::black();
        }

        auto nearest_hit = trace_single_ray(ray);

        if (!nearest_hit) {
            auto sun_exposure = (1.0 - dot(ray.get_normal(), m_sunlight_normal)) / 2.0;
            sun_exposure = sun_exposure >= 0.999 ? 1.0 : sun_exposure / 2.0;
            return Color::weighted_blend(m_sky_color, Color::white(), 1.0 - sun_exposure, sun_exposure);
        }

        // Oh my God, someone fix this unnecessary mutual recursion!
        // Also here applyes reflection by substractive color mixing,
        // while on the other function it uses additive.
        return Color::substractive_mix(nearest_hit->color(), trace_ray(Ray(
            nearest_hit->point(),
            Normal3(ray.get_normal() + nearest_hit->normal() *
                (std::fabs(dot(ray.get_normal(), nearest_hit->normal())) * 2))
        ), reflect - 1));
    }

    std::vector<Vec3> hemisphere_points = ([]() -> auto {
        const uint32_t point_count = 100;
        std::vector<uint32_t> range(point_count);
        std::vector<Vec3> points(point_count);
        std:iota(range.begin(), range.end(), 0);
        std::transform(range.begin(), range.end(), points.begin(), [=](uint32_t i){
            auto uv = hammersley::hammersley2d(i, point_count);
            return Vec3(hammersley::hemispheresample_uniform(uv[0], uv[1]));
        });
        return points;
    })();

    Color Scene::trace_ray(const Ray &ray, int reflect) const {
        if (reflect == 0) {
            return Color::black();
        }

        auto nearest_hit = trace_single_ray(ray);

        if (!nearest_hit) {
            auto sun_exposure = (1.0 - dot(ray.get_normal(), m_sunlight_normal)) / 2.0;
            sun_exposure = sun_exposure >= 0.999 ? 1.0 : sun_exposure / 2.0;
            return Color::weighted_blend(m_sky_color, Color::white(), 1.0 - sun_exposure, sun_exposure);
        }

        auto base_color = nearest_hit->color();
        auto reflection_color = trace_and_bounce_ray(Ray(
            nearest_hit->point(),
            Normal3(ray.get_normal() + nearest_hit->normal() * (std::fabs(dot(ray.get_normal(), nearest_hit->normal())) * 2))
        ), reflect - 1);

        bool direct_light = !trace_single_ray(Ray(
            nearest_hit->point(),
            Normal3(m_sunlight_normal * -1.0)
        ));

        if (direct_light) {
            // TODO: Note for future me: base_color is paint, while reflection is light,
            // this should be implemented by substractive color mixing.
            return Color::weighted_blend(base_color, reflection_color, 2.0, 1.0);
        }

        auto orthonormal_matrix = normal_to_orthonormal_matrix(
            nearest_hit->normal(), nearest_hit->normal().to_orthogonal()
        );
        // WTF?
        std::rotate(orthonormal_matrix.begin(), orthonormal_matrix.begin() + 1, orthonormal_matrix.end());
        std::vector<Color> diffuse_light_rays(hemisphere_points.size());std::transform(
            hemisphere_points.begin(),
            hemisphere_points.end(),
            diffuse_light_rays.begin(),
            [&](const auto &hemisphere_point) -> auto {
                return trace_and_bounce_ray(Ray(
                    nearest_hit->point(),
                    Normal3(dot(hemisphere_point, orthonormal_matrix))
                ), 1);
        });
        auto diffuse_light = Color::blend(diffuse_light_rays);
        // TODO: Note for future me: base_color is paint, while diffuse_light
        // **and** reflection are light, lights should be added, and then applied
        // to the object by substractive color mixing.
        return Color::weighted_blend(Color::substractive_mix(base_color, diffuse_light), reflection_color, 2.0, 1.0);
    }

    void Camera::set_orientation(const std::array<double, 3> &orientation) {
        double horizontal_length = std::cos(orientation[0]);
        double yaw_cos = std::cos(orientation[1]);
        double yaw_sin = std::sin(orientation[1]);
        Normal3 lookat(std::array{
            horizontal_length * yaw_cos,
            horizontal_length * yaw_sin,
            std::sin(orientation[0])
        });
        Normal3 left(std::array{
            -yaw_sin,
            yaw_cos,
            0.0
        });
        m_view_transform = normal_to_orthonormal_matrix(lookat, left);
        m_orientation = orientation;
    }

    std::vector<Color> Camera::render_scene(const Scene &scene, int width, int height) {
        std::vector<Color> frame(width * height);

        for (int y = 0; y < height; ++y) {
            double surface_y = m_plane_height * (0.5 - static_cast<double>(y) / height);

            for (int x = 0; x < width; ++x) {
                double surface_x = m_plane_width * (static_cast<double>(x) / width - 0.5);
                frame[y * width + x] = scene.trace_ray(Ray(
                    m_position,
                    dot(Normal3(std::array{m_focal_distance, -surface_x, surface_y}), m_view_transform)
                ), 10);
            }
        }

        return frame;
    }

    Color Camera::test_point(const Scene &scene, int width, int height, int x, int y) {
        double surface_y = m_plane_height * (0.5 - static_cast<double>(y) / height);
        double surface_x = m_plane_width * (static_cast<double>(x) / width - 0.5);
        return scene.trace_ray(Ray(
            m_position,
            dot(Normal3(std::array{m_focal_distance, -surface_x, surface_y}), m_view_transform)
        ), 10);
    }
} // namespace joytracer
