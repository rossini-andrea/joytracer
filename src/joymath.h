#pragma once
#include <cmath>
#include <algorithm>
#include <functional>
#include <numeric>
#include <array>

namespace joytracer {
    const double epsilon = 0.000000001;

    template<class T, std::size_t N>
    constexpr T dot(const std::array<T, N> &a,
        const std::array<T, N> &b) {
        return std::inner_product(a.begin(), a.end(), b.begin(), T(0));
    }

    template<class T>
    constexpr std::array<T, 3> cross(const std::array<T, 3> &a,
        const std::array<T, 3> &b) {
        return {
            a[1]*b[2] - a[2]*b[1],
            a[2]*b[0] - a[0]*b[2],
            a[0]*b[1] - a[1]*b[0]
        };
    }

    template<class T, std::size_t N>
    constexpr std::array<T, N> operator+(const std::array<T, N> &a,
        const std::array<T, N> &b) {

        std::array<T, N> result;
        std::transform(a.begin(), a.end(), b.begin(),
            result.begin(), std::plus<T>());
        return result;
    }

    template<class T, std::size_t N>
    constexpr std::array<T, N> operator-(const std::array<T, N> &a,
        const std::array<T, N> &b) {

        std::array<T, N> result;
        std::transform(a.begin(), a.end(), b.begin(),
            result.begin(), std::minus<T>());
        return result;
    }

    template<class T, std::size_t N>
    constexpr std::array<T, N> operator*(const std::array<T, N> &a,
        const T &b) {

        std::array<T, N> result;
        std::transform(a.begin(), a.end(), result.begin(),
            [=](T x) -> T { return x * b; });
        return result;
    }

    template<class T, std::size_t N>
    constexpr std::array<T, N> operator*(const std::array<T, N> &a,
        const std::array<T, N> &b) {

        std::array<T, N> result;
        std::transform(a.begin(), a.end(), b.begin(),
            result.begin(), std::multiplies<T>());
        return result;
    }

    template<class T, std::size_t N>
    constexpr std::array<T, N> operator/(const std::array<T, N> &a,
        const T &b) {

        std::array<T, N> result;
        std::transform(a.begin(), a.end(), result.begin(),
            [=](T x) -> T { return x / b; });
        return result;
    }

    template<class T, std::size_t N>
    constexpr T vector_length(const std::array<T, N> &a) {
        return std::sqrt(std::accumulate(a.begin(), a.end(), (T)0,
            [=](T total, T x) -> T { return total + x * x; }));
    }

    template<class T, std::size_t N>
    constexpr auto normalize(const std::array<T, N> &a) {
        T length = vector_length(a);
        std::array<T, N> result;
        std::transform(a.begin(), a.end(), result.begin(),
            [=](T x) -> T { return x / length; });
        return result;
    }

    template<class T>
    constexpr std::array<T, 3> dot(const std::array<T, 3> &vec,
        const std::array<std::array<T, 3>, 3> &matrix) {
        std::array<std::array<T, 3>, 3> scaled_matrix;
        std::transform(
            vec.begin(), vec.end(), matrix.begin(), scaled_matrix.begin(),
            [](const auto &a, const auto &b) {
                return b * a;
            }
        );
        return std::accumulate(scaled_matrix.begin(), scaled_matrix.end(),
            std::array<T, 3>{0, 0, 0},
            [](const auto &a, const auto &b) {
                return a + b;
            }
        );
    }

    /*
    * Non normalized vector.
    */
    using Vec3 = std::array<double, 3>;

    /*
    * 3x3 matrix.
    */
   using Mat3x3 = std::array<Vec3, 3>;

    /*
    * A normalized vector.
    */
    class Normal3: public Vec3 {
    public:
        constexpr explicit Normal3(const Normal3 &vector):
            Vec3(vector)
        {

        }

        constexpr explicit Normal3(const Vec3 &vector):
            Vec3(normalize(vector))
        {

        }

        constexpr Normal3 to_orthogonal() const {
            return Normal3 (
                operator[](1) - operator[](2),
                -operator[](0) + operator[](2),
                operator[](0) - operator[](1)
            );
        }
    private:
        constexpr Normal3(double x, double y, double z): Vec3{x,y,z} { }
    };

    constexpr Normal3 dot(const Normal3 &vec, const Mat3x3 &matrix) {
        return Normal3(dot((Vec3)vec, matrix));
    }

    constexpr Mat3x3 normal_to_orthonormal_matrix(
        const Normal3 &first_normal,
        const Normal3 &second_normal) {
        Normal3 third_normal(cross(first_normal, second_normal));
        return Mat3x3 {
            first_normal,
            cross(third_normal, first_normal),
            third_normal
        };
    }

    /*
    * Type for a color.
    */
    class Color {
    private:
        std::array<double, 3> m_value;
        constexpr Color(const std::array<double, 3> &rgb) :
            m_value(rgb) {}
    public:
        constexpr Color(): m_value() {}
        std::array<double, 3> to_rgb() { return m_value; }

        static constexpr Color from_rgb(const std::array<double, 3> &rgb) {
            return Color(rgb);
        }

        static constexpr Color black() { return Color(std::array{0.0, 0.0, 0.0}); }
        static constexpr Color white() { return Color(std::array{1.0, 1.0, 1.0}); }

        static Color blend(
            const std::vector<Color> &colors
        );
        static constexpr Color weighted_blend(
            const Color &first_color, const Color &second_color,
            const double first_weight, const double second_weight
        );
        static constexpr Color substractive_mix(
            const Color &first_color, const Color &second_color
        );
    };

    constexpr Color Color::weighted_blend(
        const Color &first_color, const Color &second_color,
        const double first_weight, const double second_weight
    ) {
        return from_rgb((
            first_color.m_value * first_weight +
            second_color.m_value * second_weight
        ) / (first_weight + second_weight));
    }

    constexpr Color Color::substractive_mix(
        const Color &first_color, const Color &second_color
    ) {
        return from_rgb(
            first_color.m_value * second_color.m_value
        );
    }
}
