#include <algorithm>
#include <random>

#include "hammersley.h"
#include "joymath.h"
#include "joytracer.h"

namespace joytracer {
    class RandomHammersleyPoint {
        private:
            std::vector<std::array<double, 3>> m_points;
            std::random_device m_random_device;
            std::mt19937 m_random_engine;
            std::uniform_int_distribution<int> m_random_distribution;
        public:
            RandomHammersleyPoint(int max_points) :
                m_points(max_points),
                m_random_device(),
                m_random_engine(m_random_device()),
                m_random_distribution(0, max_points - 1) {
                std::vector<int> range(max_points);
                std::iota(range.begin(), range.end(), 0);
                std::transform(range.begin(), range.end(), m_points.begin(), [&](auto &i) -> auto{
                    auto uv = hammersley::hammersley2d(i, max_points);
                    return hammersley::hemispheresample_uniform(uv[0], uv[1]);
                });
            }

            std::array<double, 3> operator()() {
                return m_points.at(m_random_distribution(m_random_engine));
            }
    };

    std::optional<HitPoint> project_ray_on_plane_frontface(
        const Ray &ray,
        const std::array<double, 3> &plane_origin,
        const std::array<double, 3> &plane_normal) {
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
            const std::array<std::array<double, 3>, 3> &vertices,
            const std::array<double, 3> &color
        ) : m_vertices(vertices), m_color(color),
            m_normal(normalize(cross(
                vertices[1] - vertices[0],
                vertices[2] - vertices[1]
            ))) {
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
        auto projection = project_ray_on_plane_frontface(ray, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0});

        if (!projection) {
            return std::nullopt;
        }

        auto hit_point = projection->point();
        long is_x_odd = static_cast<long>(floorf(hit_point[0])) & 1;
        long is_y_odd = static_cast<long>(floorf(hit_point[1])) & 1;
        return HitResult(projection->distance(), hit_point,
            {0.0, 0.0, 1.0},
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
            normalize(hit_point - m_center),
            m_color);
    }

    std::optional<HitResult> Scene::trace_single_ray(const Ray &ray) const {
        std::vector<std::optional<HitResult>> hits;
        std::transform(m_surfaces.begin(), m_surfaces.end(),
            std::back_inserter(hits),
            [=] (auto &s) -> auto { return s->hit_test(ray); });
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

    std::array<double, 3> Scene::trace_and_bounce_ray(const Ray &ray, int reflect) const {
        if (reflect == 0) {
            return {0,0,0};
        }

        auto nearest_hit = trace_single_ray(ray);

        if (!nearest_hit) {
            auto sun_exposure = (1.0 - dot(ray.get_normal(), m_sunlight_normal)) / 2.0;
            sun_exposure = sun_exposure >= 0.999 ? 1.0 : sun_exposure / 2.0;
            return m_sky_color * (1.0 - sun_exposure) + std::array{1.0, 1.0, 1.0} * sun_exposure;
        }

        return (nearest_hit->color() * trace_ray(Ray(
            nearest_hit->point(),
            ray.get_normal() + nearest_hit->normal() * (std::fabs(dot(ray.get_normal(), nearest_hit->normal())) * 2)
        ), reflect - 1));
    }

    RandomHammersleyPoint random_hemisphere_point(1000);
    std::vector<std::array<double, 3>> hemisphere_points = ([]() -> auto {
        uint32_t i(0);
        std::vector<std::array<double, 3>> points(100);
        std::generate_n(points.begin(), points.size(), [&](){
            auto uv = hammersley::hammersley2d(i, points.size()); ++i;
            return hammersley::hemispheresample_uniform(uv[0], uv[1]);
        });
        return points;
    })();

    std::array<double, 3> Scene::trace_ray(const Ray &ray, int reflect) const {
        if (reflect == 0) {
            return {0,0,0};
        }

        auto nearest_hit = trace_single_ray(ray);

        if (!nearest_hit) {
            auto sun_exposure = (1.0 - dot(ray.get_normal(), m_sunlight_normal)) / 2.0;
            sun_exposure = sun_exposure >= 0.999 ? 1.0 : sun_exposure / 2.0;
            return m_sky_color * (1.0 - sun_exposure) + std::array{1.0, 1.0, 1.0} * sun_exposure;
        }

        auto base_color = nearest_hit->color();
        auto reflection_color = trace_and_bounce_ray(Ray(
            nearest_hit->point(),
            ray.get_normal() + nearest_hit->normal() * (std::fabs(dot(ray.get_normal(), nearest_hit->normal())) * 2)
        ), reflect - 1);

        bool direct_light = !trace_single_ray(Ray(
            nearest_hit->point(),
            m_sunlight_normal * -1.0
        ));

        if (direct_light) {
            return (base_color * 2.0 + reflection_color) / 3.0;
        }

        auto orthonormal_matrix = normal_to_orthonormal_matrix(nearest_hit->normal(), {1.0, 0.0, 0.0});
        std::rotate(orthonormal_matrix.begin(), orthonormal_matrix.begin() + 1, orthonormal_matrix.end());
        auto diffuse_light = std::accumulate(hemisphere_points.begin(), hemisphere_points.end(), std::array{0.0, 0.0, 0.0},
            [&](const auto &accum, const auto &hemisphere_point) -> auto {
                return accum + trace_and_bounce_ray(Ray(
                    nearest_hit->point(),
                    dot(hemisphere_point, orthonormal_matrix)
                ), 1);
        }) / static_cast<double>(hemisphere_points.size());

        return ((base_color * diffuse_light) * 2.0 + reflection_color) / 3.0;
    }

    void Camera::set_orientation(const std::array<double, 3> &orientation) {
        double horizontal_length = std::cos(orientation[0]);
        double yaw_cos = std::cos(orientation[1]);
        double yaw_sin = std::sin(orientation[1]);
        auto lookat = normalize<double, 3>({
            horizontal_length * yaw_cos,
            horizontal_length * yaw_sin,
            std::sin(orientation[0])
        });
        auto left = normalize<double, 3>({
            -yaw_sin,
            yaw_cos,
            0.0
        });
        m_view_transform = normal_to_orthonormal_matrix(lookat, left);
        m_orientation = orientation;
    }

    std::vector<std::array<double, 3>> Camera::render_scene(const Scene &scene, int width, int height) {
        std::vector<std::array<double, 3>> frame(width * height);

        for (int y = 0; y < height; ++y) {
            double surface_y = m_plane_height * (0.5 - static_cast<double>(y) / height);

            for (int x = 0; x < width; ++x) {
                double surface_x = m_plane_width * (static_cast<double>(x) / width - 0.5);
                frame[y * width + x] = scene.trace_ray(Ray(
                    m_position,
                    dot(normalize(std::array{m_focal_distance, -surface_x, surface_y}), m_view_transform)
                ), 10);
            }
        }

        return frame;
    }

    std::array<double, 3> Camera::test_point(const Scene &scene, int width, int height, int x, int y) {
        double surface_y = m_plane_height * (0.5 - static_cast<double>(y) / height);
        double surface_x = m_plane_width * (static_cast<double>(x) / width - 0.5);
        return scene.trace_ray(Ray(
            m_position,
            dot(normalize(std::array{m_focal_distance, -surface_x, surface_y}), m_view_transform)
        ), 10);
    }
} // namespace joytracer
