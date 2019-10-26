#pragma once
#include <array>
#include <vector>
#include <optional>
#include <memory>

/*
* Main namespace for the app.
*/
namespace joytracer {
    /*
    * A ray cast out into the scene.
    */
    class Ray {
    private:
        std::array<double, 3> m_origin;
        std::array<double, 3> m_normal;
    public:
        Ray(const std::array<double, 3> &origin,
        const std::array<double, 3> &normal) :
        m_origin(origin),
        m_normal(normal) {}

        const std::array<double, 3> &get_origin() const {
            return m_origin;
        }

        const std::array<double, 3> &get_normal() const {
            return m_normal;
        }
    };

    /*
    * Geometrical information about a hit result
    */
    class HitPoint {
    private:
        double m_distance;
        std::array<double, 3> m_point;
    public:
        HitPoint(
            double distance,
            const std::array<double, 3> &point
        ) : m_distance(distance), m_point(point) {}

        double distance() const {
            return m_distance;
        }

        const std::array<double, 3> &point() const {
            return m_point;
        }
    };

    /*
    * The result of casting a ray and successfully hitting a surface.
    */
    class HitResult {
    private:
        double m_distance;
        std::array<double, 3> m_point;
        std::array<double, 3> m_color;
    public:
        HitResult(
            double distance,
            const std::array<double, 3> &point,
            const std::array<double, 3> &color
        ) : m_distance(distance), m_point(point), m_color(color) {}

        double distance() const {
            return m_distance;
        }

        const std::array<double, 3> &point() const {
            return m_point;
        }

        const std::array<double, 3> &color() const {
            return m_color;
        }
    };

    /*
    * Base class for all surfaces.
    */
    class Surface {
    public:
        virtual ~Surface() = default;
        virtual std::optional<HitResult> hit_test(const Ray &ray) const = 0;
    };

    /*
    * A triangle in 3D space.
    */
    class Triangle : public Surface {
    private:
        std::array<std::array<double, 3>, 3> m_vertices;
        std::array<double, 3> m_color;
        std::array<double, 3> m_normal;
    public:
        Triangle(
            const std::array<std::array<double, 3>, 3> &vertices,
            const std::array<double, 3> &color
        );
        std::optional<HitResult> hit_test(const Ray &ray) const override;
    };

    /*
    * An infinite surface facing upward with fixed origin at `0,0,0`.
    */
    class Floor : public Surface {
    public:
        Floor() {}
        ~Floor() {}
        std::optional<HitResult> hit_test(const Ray &ray) const override;
    };

    /*
    * The sphere shape.
    */
    class Sphere : public Surface {
    private:
        double m_radius;
        std::array<double, 3> m_center;
        std::array<double, 3> m_color;
    public:
        Sphere(double radius, std::array<double, 3> center, std::array<double, 3> color) :
            m_radius(radius),
            m_center(center),
            m_color(color)
        {}
        ~Sphere() {}
        std::optional<HitResult> hit_test(const Ray &ray) const override;
    };

    /*
    * The scene, holding all models and surfaces.
    */
    class Scene {
    private:
        std::vector<std::unique_ptr<Surface>> m_surfaces;
        std::array<double, 3> m_sky_color;

    public:
        Scene(
            std::vector<std::unique_ptr<Surface>> surfaces,
            const std::array<double, 3> &sky_color
        ) : m_surfaces(std::move(surfaces)),
        m_sky_color(sky_color) {}
        std::array<double, 3> trace_ray(const Ray &ray) const;

    };

    /*
    * Stores the projection settings for looking into the scene,
    * and provides the rendering functionality.
    */
    class Camera {
    private:
        std::array<double, 3> m_position;
        std::array<double, 3> m_orientation;
        std::array<double, 3> m_lookat;
        std::array<double, 3> m_camera_up;
        double m_focal_distance;
        double m_plane_width, m_plane_height;
    public:
        void set_position(const std::array<double, 3> &position) {
            m_position = position;
        }

        // Set orientation as `{pitch, yaw, roll}`
        void set_orientation(const std::array<double, 3> &orientation);

        void set_focal_distance(double focal_distance) {
            m_focal_distance = focal_distance;
        }

        void set_plane_size(double width, double height) {
            m_plane_width = width; m_plane_height = height;
        }

        std::vector<std::array<double, 3>> render_scene(const Scene &scene, int width, int height);
    };
}
