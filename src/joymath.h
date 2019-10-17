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
        T result = 0;

        for(std::size_t i = 0; i < N; ++i) {
            result += a[i] * b[i];
        }

        return result;
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
    auto normalize(const std::array<T, N> &a) {
        T length = std::sqrt(std::accumulate(a.begin(), a.end(), (T)0,
            [=](T total, T x) -> T { return total + x * x; }));
        std::array<T, N> result;
        std::transform(a.begin(), a.end(), result.begin(),
            [=](T x) -> T { return x / length; });
        return result;
    }
}
