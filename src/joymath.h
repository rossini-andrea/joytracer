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
    constexpr std::array<std::array<T, 3>, 3> normal_to_orthonormal_matrix(const std::array<T, 3> &n) {
        auto xy_length = std::sqrt(n[0]*n[0] + n[1]*n[1]);
        return std::array{
            std::array<T, 3>{n[1] / xy_length,     -n[0] / xy_length,      0},
            std::array<T, 3>{n[0]*n[2] / xy_length, n[1]*n[2] / xy_length, -xy_length},
            n
        };
    }

    template<class T>
    constexpr std::array<T, 3> dot(const std::array<T, 3> &vec,
        const std::array<std::array<T, 3>, 3> &matrix) {

        std::array<T, 3> result;
        uint32_t i(0);
        std::generate_n(result.begin(), 3, [&](){
            uint32_t i1 = i++;
            return std::inner_product(vec.begin(), vec.end(), matrix.begin(), T(0),
                [](const auto &a, const auto &b){ return a + b; },
                [=](const auto &vec_element, const auto &mat_row){ return vec_element * mat_row[i1]; }
            );
        });

        return result;
    }
}
