#pragma once
#include <array>
#include <vector>
#include <optional>
#include <memory>
#include <variant>

#include "joymath.h"

/*
* Main namespace for the app.
*/
namespace joytracer {
    /*
    * A ray cast out into the scene.
    */
    class Ray {
    private:
        Vec3 m_origin;
        Normal3 m_normal;
    public:
        Ray(const Vec3 &origin,
        const Normal3 &normal) :
        m_origin(origin),
        m_normal(normal) {}

        const Vec3 &get_origin() const {
            return m_origin;
        }

        const Normal3 &get_normal() const {
            return m_normal;
        }
    };

    /*
    * Geometrical information about a hit result
    */
    class HitPoint {
    private:
        double m_distance;
        Vec3 m_point;
    public:
        HitPoint(
            double distance,
            const Vec3 &point
        ) : m_distance(distance), m_point(point) {}

        double distance() const {
            return m_distance;
        }

        const Vec3 &point() const {
            return m_point;
        }
    };

    /*
    * The result of casting a ray and successfully hitting a surface.
    */
    class HitResult {
    private:
        double m_distance;
        Vec3 m_point;
        Normal3 m_normal;
        Color m_color;
    public:
        HitResult(
            double distance,
            const Vec3 &point,
            const Normal3 &normal,
            const Color &color
        ) : m_distance(distance), m_point(point), m_normal(normal), m_color(color) {}

        double distance() const {
            return m_distance;
        }

        const Vec3 &point() const {
            return m_point;
        }

        const Normal3 &normal() const {
            return m_normal;
        }

        const Color &color() const {
            return m_color;
        }
    };

    /*
    * A triangle in 3D space.
    */
    class Triangle {
    private:
        std::array<Vec3, 3> m_vertices;
        Color m_color;
        Normal3 m_normal;
    public:
        Triangle(
            const std::array<Vec3, 3> &vertices,
            const Color &color
        );
        std::optional<HitResult> hit_test(const Ray &ray) const;
    };

    /*
    * An infinite surface facing upward with fixed origin at `0,0,0`.
    */
    class Floor {
    public:
        Floor() {}
        ~Floor() {}
        std::optional<HitResult> hit_test(const Ray &ray) const;
    };

    /*
    * The sphere shape.
    */
    class Sphere {
    private:
        double m_radius;
        Vec3 m_center;
        Color m_color;
    public:
        Sphere(double radius, Vec3 center, Color color) :
            m_radius(radius),
            m_center(center),
            m_color(color)
        {}
        ~Sphere() {}
        std::optional<HitResult> hit_test(const Ray &ray) const;
    };

    /*
    * Any kind of surface.
    */
    using Surface = std::variant<Triangle, Floor, Sphere>;

    /*
    * A visitor to call the hit_test function of a Surface.
    */
    class HitTestVisitor {
    public:
        template<typename TSurface>
        std::optional<HitResult> operator()(const TSurface &surface) {
            return surface.hit_test(m_ray);
        }

        HitTestVisitor(const Ray& ray) : m_ray(ray) {}
    private:
        const Ray& m_ray;
    };

    /*
    * The scene, holding all models and surfaces.
    */
    class Scene {
    private:
        std::vector<Surface> m_surfaces;
        Color m_sky_color;
        Normal3 m_sunlight_normal;

        std::optional<HitResult> trace_single_ray(const Ray &ray) const;
        Color trace_and_bounce_ray(const Ray &ray, int reflect) const;
    public:
        Scene(
            std::vector<Surface> surfaces,
            const Color &sky_color,
            const Normal3 &sunlight_normal
        ) : m_surfaces(std::move(surfaces)),
        m_sky_color(sky_color),
        m_sunlight_normal(sunlight_normal) {}
        Color trace_ray(const Ray &ray, int reflect) const;
    };

    /*
    * Stores the projection settings for looking into the scene,
    * and provides the rendering functionality.
    */
    class Camera {
    private:
        Vec3 m_position;
        std::array<double, 3> m_orientation;
        std::array<std::array<double, 3>, 3> m_view_transform;
        double m_focal_distance;
        double m_plane_width, m_plane_height;
    public:
        void set_position(const Vec3 &position) {
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

        std::vector<Color> render_scene(const Scene &scene, int width, int height);
        Color test_point(const Scene &scene, int width, int height, int x, int y);
    };
}
