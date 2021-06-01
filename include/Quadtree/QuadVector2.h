/* Adapted From https://github.com/pvigier/Quadtree */

#pragma once

namespace quadtree
{

    template <typename T>
    class QuadVector2
    {
    public:
        T x;
        T y;

        constexpr QuadVector2<T>(T X = 0, T Y = 0) noexcept : x(X), y(Y)
        {
        }

        constexpr QuadVector2<T> &operator+=(const QuadVector2<T> &other) noexcept
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr QuadVector2<T> &operator/=(T t) noexcept
        {
            x /= t;
            y /= t;
            return *this;
        }
    };

    template <typename T>
    constexpr QuadVector2<T> operator+(QuadVector2<T> lhs, const QuadVector2<T> &rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }

    template <typename T>
    constexpr QuadVector2<T> operator/(QuadVector2<T> vec, T t) noexcept
    {
        vec /= t;
        return vec;
    }

}