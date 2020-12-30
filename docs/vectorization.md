# Vectorization

## About

This document gathers some ideas and experiments results for vectorizing the
algorithms.

## First steps

The investigation is performed with the MSVC compiler, since in my very biased
opinion "if it vectorizes on MS, it vectorizes on everything". To not pollute
the `CMakeLists.txt`, I added the experimental CMAKE_CXX_FLAGS "/Qvec-report:2"
to the command line, or to the editor settings.

The first Qvec report had many false positives on loops involving floating
point numbers, so to limit the "not vectorized" messages I added the `/fp:fast`
option (`-ffast-math` for clang). When I started the project I wanted to learn
the standard library and already had vectorization in mind, so the benefit was
immediate.

## The new Vec3*Matrix product

I then decided to implement a new version of Vec3*Matrix product, I never liked
how the code looked like. So, to make code appear a bit more readable, I came
up with this new implementation:

```c++
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
```

And here is the old one, for reference:

```c++
template<class T>
constexpr std::array<T, 3> old_dot(const std::array<T, 3> &vec,
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
```

MSVC reported to vectorize the new algorithm a bit more, then I isolated the two
implementations in a Google benchmark project and ran comparisons. `BM_dot` is
the new version, `BM_dot_old` the previous one.

The benchmark machine (Windows10):

```txt
Run on (8 X 2798 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 8192 KiB (x1)
```

Benchmark compiled with MSVC:

Benchmark             |   Time|             CPU|   Iterations
----------------------|-------|----------------|-------------
BM_dot/0/0            |47.3 ns|         48.1 ns|     14933333
BM_dot/0/0            |47.3 ns|         47.1 ns|     14933333
BM_dot/0/1            |47.6 ns|         47.6 ns|     14451613
BM_dot/0/8            |47.3 ns|         47.1 ns|     14933333
BM_dot/0/11           |47.4 ns|         47.6 ns|     14451613
BM_old_dot/0/0        |15.1 ns|         15.0 ns|     44800000
BM_old_dot/0/0        |16.0 ns|         16.0 ns|     44800000
BM_old_dot/0/1        |15.1 ns|         15.0 ns|     44800000
BM_old_dot/0/8        |15.4 ns|         15.4 ns|     49777778
BM_old_dot/0/11       |15.1 ns|         15.0 ns|     44800000

Benchmark compiled with clang for Visual Studio:

Benchmark             |   Time|             CPU|   Iterations
----------------------|-------|----------------|-------------
BM_dot/0/0            |45.7 ns|         46.5 ns|     15448276
BM_dot/0/0            |45.7 ns|         45.9 ns|     16000000
BM_dot/0/1            |45.7 ns|         46.5 ns|     15448276
BM_dot/0/8            |45.6 ns|         45.0 ns|     14933333
BM_dot/0/11           |45.7 ns|         45.5 ns|     15448276
BM_old_dot/0/0        |14.5 ns|         14.4 ns|     49777778
BM_old_dot/0/0        |14.6 ns|         15.1 ns|     56000000
BM_old_dot/0/1        |14.5 ns|         14.6 ns|     44800000
BM_old_dot/0/8        |14.5 ns|         14.3 ns|     44800000
BM_old_dot/0/11       |14.5 ns|         14.4 ns|     49777778

The second benchmark machine (VirtualBox running Debian test):

```txt
Run on (2 X 2798.01 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x2)
  L1 Instruction 32 KiB (x2)
  L2 Unified 256 KiB (x2)
  L3 Unified 8192 KiB (x2)
```

Benchmark compiled with clang 11:

Benchmark             |   Time|             CPU|   Iterations
----------------------|-------|----------------|-------------
BM_dot/0/0            |7.15 ns|         7.13 ns|     96008000
BM_dot/0/1            |7.02 ns|         7.01 ns|     98552371
BM_dot/0/8            |7.36 ns|         7.35 ns|     96676554
BM_dot/0/11           |7.46 ns|         7.44 ns|     91866892
BM_old_dot/0/0        |7.82 ns|         7.81 ns|     89314991
BM_old_dot/0/1        |7.54 ns|         7.54 ns|     91448735
BM_old_dot/0/8        |7.82 ns|         7.82 ns|     86991100
BM_old_dot/0/11       |6.70 ns|         6.70 ns|    101995463

Benchmark compiled with gcc 10.2.1:

Benchmark             |   Time|             CPU|   Iterations
----------------------|-------|----------------|-------------
BM_dot/0/0            |9.75 ns|         9.73 ns|     71439345
BM_dot/0/1            |9.89 ns|         9.86 ns|     67490416
BM_dot/0/8            |9.82 ns|         9.77 ns|     71127793
BM_dot/0/11           |9.73 ns|         9.72 ns|     71638834
BM_old_dot/0/0        |9.76 ns|         9.74 ns|     70876127
BM_old_dot/0/1        |9.78 ns|         9.78 ns|     71139495
BM_old_dot/0/8        |9.77 ns|         9.75 ns|     69652206
BM_old_dot/0/11       |9.77 ns|         9.73 ns|     71244249

My take on the results is:

* The old version is better on Windows both with clang and MSVC, probably
  because the functions I use in MS's std have performance differencies.
* Linux compilers, and probably their std implementations are blazingly faster,
  and give more stable results, revealing my two implementation aren't that
  different.

Next thing to do is to go back to Windows, and try with clang and gcc in MSYS/MinGW.
Which, for clang 10 gives:

Benchmark             |   Time|             CPU|   Iterations
----------------------|-------|----------------|-------------
BM_dot/0/0            |7.45 ns|         7.53 ns|    112000000
BM_dot/0/1            |6.98 ns|         6.98 ns|    112000000
BM_dot/0/8            |7.35 ns|         7.32 ns|     89600000
BM_dot/0/11           |7.37 ns|         7.32 ns|     89600000
BM_old_dot/0/0        |8.59 ns|         8.54 ns|     89600000
BM_old_dot/0/1        |9.06 ns|         9.00 ns|     74666667
BM_old_dot/0/8        |8.27 ns|         8.16 ns|     74666667
BM_old_dot/0/11       |8.14 ns|         8.20 ns|     89600000

And for gcc 10.1.0:

Benchmark             |   Time|             CPU|   Iterations
----------------------|-------|----------------|-------------
BM_dot/0/0            |11.2 ns|         11.2 ns|     64000000
BM_dot/0/1            |11.0 ns|         11.2 ns|     64000000
BM_dot/0/8            |11.0 ns|         10.5 ns|     64000000
BM_dot/0/11           |10.5 ns|         10.5 ns|     64000000
BM_old_dot/0/0        |9.89 ns|         9.84 ns|     74666667
BM_old_dot/0/1        |9.93 ns|         9.77 ns|     64000000
BM_old_dot/0/8        |9.88 ns|         9.77 ns|     64000000
BM_old_dot/0/11       |9.87 ns|         9.52 ns|     64000000

So we have a winner, right? I am planning to keep support for Visual Studio, but
actively use clang. And I will keep the new implementation since it is no
different from the older, but more eye-appealing.

## Final results

After making small code changes, adding vectorization options, and reverting to
MinGW clang, the initial frame render dropped to half the time.

## What's next?

This branch allowed me to review some old code that may need to be simplified.
The `Vec3` class may be declared with a simple `using` allowing to:

* Replace constructors with initializers.
* Remove operators duplication.
* Simplify `Normal3` class.

Another thing that comes to mind is to totally change architecture. Currently the
project doesn't have a rendering pipeline, it justs runs code in "logical" order.
