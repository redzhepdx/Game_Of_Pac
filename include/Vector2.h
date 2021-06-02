#ifndef __VECTOR2D__
#define __VECTOR2D__

#include <GLFW/glfw3.h>

#include <iostream>
#include <algorithm>
#include <memory>
#include <cmath>

template<typename T, std::uint8_t iterations = 2>
inline T inverse_sqrt(T x) {
    static_assert(std::is_floating_point<T>::value, "T must be floating point");
    static_assert(iterations > 0, "itarations must greater than 0");

    typedef typename std::conditional<sizeof(T) == 8, std::int64_t, std::int32_t>::type Tint;

    T y = x;
    T x2 = y * 0.5;

    Tint i = *(Tint *) &y;

    i = (sizeof(T) == 8 ? 0x5fe6eb50c7b537a9 : 0x5f3759df) - (i >> 1);

    y = *(T *) &i;
    y = y * (1.5 - (x2 * y * y));

    // if (iterations == 2)
    for (std::uint8_t iteration = 1; iteration < iterations; ++iteration) {
        y = y * (1.5 - (x2 * y * y));
    }

    return y;
}

template<typename T>
class Vector2 {
public:
    T x;
    T y;

public:

    Vector2();

    Vector2(T x, T y);

    Vector2(const Vector2<T> &other);

    ~Vector2() = default;

    T magnitude() const;

    Vector2<T> normalize();

    [[nodiscard]] float angle() const;

    Vector2<T> operator+(const Vector2<T> &other) const;

    Vector2<T> operator-(const Vector2<T> &other) const;

    Vector2<T> &operator=(const Vector2<T> &other);

    Vector2<T> operator*(T value) const;

    void operator+=(const Vector2<T> &other);

    void operator-=(const Vector2<T> &other);

    bool operator==(const Vector2<T> &other) const;

    bool operator!=(const Vector2<T> &other) const;
};

#endif
