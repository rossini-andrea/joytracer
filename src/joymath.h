#pragma once
#include <cmath>
#include <algorithm>
#include <functional>
#include <numeric>

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

        std::array<uint32_t, 3> i;
        std::array<T, 3> result;
        std::iota(i.begin(), i.end(), 0);
        std::transform(i.begin(), i.end(), result.begin(), [&](uint32_t column){
            return std::inner_product(vec.begin(), vec.end(), matrix.begin(), T(0),
                [](const auto &a, const auto &b){ return a + b; },
                [=](const auto &vec_element, const auto &mat_row){ return vec_element * mat_row[column]; }
            );
        });

        return result;
    }

    /*
    * Non normalized vector.
    */
    class Vec3 {
    protected:
        std::array<double, 3> m_value;
    public:
        constexpr Vec3(): m_value() {}

        constexpr Vec3(const std::array<double, 3> &vector): m_value(vector) {

        }

        constexpr double operator[](size_t i) {
            return m_value[i];
        }

        constexpr const std::array<double, 3>& get_value() const {
            return m_value;
        }

        constexpr Vec3 operator+(const Vec3 &other) const {
            return this->m_value + other.m_value;
        }

        constexpr Vec3 operator-(const Vec3 &other) const {
            return this->m_value - other.m_value;
        }

        constexpr Vec3 operator*(double scalar) const {
            return this->m_value * scalar;
        }

        constexpr Vec3 operator*(const Vec3 &other) const {
            return this->m_value * other.m_value;
        }

        constexpr Vec3 operator/(double scalar) const {
            return this->m_value / scalar;
        }

        constexpr auto vector_length() const {
            return joytracer::vector_length(m_value);
        }
    };

    /*
    * A normalized vector.
    */
    class Normal3: public Vec3 {
    public:
        constexpr explicit Normal3(const Vec3 &vector):
            Vec3(normalize(vector.get_value()))
        {

        }

        constexpr Normal3 to_orthogonal() const {
            return Normal3(
                m_value[1] - m_value[2],
                -m_value[0] + m_value[2],
                m_value[0] - m_value[1]
            );
        }
    private:
        constexpr Normal3(double x, double y, double z): Vec3({x,y,z}) { }
    };

    constexpr double dot(const Vec3 &a, const Vec3 &b) {
        return dot(a.get_value(), b.get_value());
    }

    constexpr Vec3 dot(const Vec3 &vec, const std::array<std::array<double, 3>, 3> &matrix) {
        return dot(vec.get_value(), matrix);
    }

    constexpr Vec3 cross(const Vec3 &a, const Vec3 &b) {
        return cross(a.get_value(), b.get_value());
    }

    // TODO: Fix this wrapper signature ASAP. I am no mathematician, but my OOP sense
    // tells me that result is normal if matrix is an orthonormal projection.
    constexpr Normal3 dot(const Normal3 &vec, const std::array<std::array<double, 3>, 3> &matrix) {
        return Normal3(dot(vec.get_value(), matrix));
    }

    constexpr std::array<std::array<double, 3>, 3> normal_to_orthonormal_matrix(
        const Normal3 &first_normal,
        const Normal3 &second_normal) {
        auto third_normal = Normal3(cross(first_normal, second_normal));
        return std::array{
            first_normal.get_value(),
            cross(third_normal, first_normal).get_value(),
            third_normal.get_value()
        };
    }
}
