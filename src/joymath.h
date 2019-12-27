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

    constexpr std::array<double, 3> normal_to_orthogonal(const std::array<double, 3> normal) {
        return std::array{
            normal[1] - normal[2],
            -normal[0] + normal[2],
            normal[0] - normal[1]
        };
    }

    constexpr std::array<std::array<double, 3>, 3> normal_to_orthonormal_matrix(
        const std::array<double, 3> &first_normal,
        const std::array<double, 3> &second_normal) {
        auto third_normal = normalize(cross(first_normal, second_normal));
        return std::array{
            first_normal,
            cross(third_normal, first_normal),
            third_normal
        };
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
}
