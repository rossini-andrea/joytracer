#pragma once
#include <cmath>
#include <algorithm>
#include <functional>
#include <numeric>

namespace joytracer {
    const double epsilon = 0.000000001;

    template<class T, std::size_t N>
    T dot(const std::array<T, N> &a,
        const std::array<T, N> &b) {
        return std::inner_product(a.begin(), a.end(), b.begin(), T(0));
    }

    template<class T>
    std::array<T, 3> cross(const std::array<T, 3> &a,
        const std::array<T, 3> &b) {
        return {
            a[1]*b[2] - a[2]*b[1],
            a[2]*b[0] - a[0]*b[2],
            a[0]*b[1] - a[1]*b[0]
        };
    }

    template<class T, std::size_t N>
    std::array<T, N> operator+(const std::array<T, N> &a,
        const std::array<T, N> &b) {

        std::array<T, N> result;
        std::transform(a.begin(), a.end(), b.begin(),
            result.begin(), std::plus<T>());
        return result;
    }

    template<class T, std::size_t N>
    std::array<T, N> operator-(const std::array<T, N> &a,
        const std::array<T, N> &b) {

        std::array<T, N> result;
        std::transform(a.begin(), a.end(), b.begin(),
            result.begin(), std::minus<T>());
        return result;
    }

    template<class T, std::size_t N>
    std::array<T, N> operator*(const std::array<T, N> &a,
        const T &b) {

        std::array<T, N> result;
        std::transform(a.begin(), a.end(), result.begin(),
            [=](T x) -> T { return x * b; });
        return result;
    }

    template<class T, std::size_t N>
    std::array<T, N> operator/(const std::array<T, N> &a,
        const T &b) {

        std::array<T, N> result;
        std::transform(a.begin(), a.end(), result.begin(),
            [=](T x) -> T { return x / b; });
        return result;
    }

    template<class T, std::size_t N>
    T vector_length(const std::array<T, N> &a) {
        return std::sqrt(std::accumulate(a.begin(), a.end(), (T)0,
            [=](T total, T x) -> T { return total + x * x; }));
    }

    template<class T, std::size_t N>
    auto normalize(const std::array<T, N> &a) {
        T length = vector_length(a);
        std::array<T, N> result;
        std::transform(a.begin(), a.end(), result.begin(),
            [=](T x) -> T { return x / length; });
        return result;
    }
}
