/* Adapted From https://github.com/pvigier/Quadtree */
#pragma once

#include "QuadVector2.h"

namespace quadtree
{

    template <typename T>
    class QuadBox
    {
    public:
        T left;
        T top;
        T width;  // Must be positive
        T height; // Must be positive

        constexpr QuadBox(T Left = 0, T Top = 0, T Width = 0, T Height = 0) noexcept : left(Left), top(Top), width(Width), height(Height)
        {
        }

        constexpr QuadBox(const QuadVector2<T> &position, const QuadVector2<T> &size) noexcept : left(position.x), top(position.y), width(size.x), height(size.y)
        {
        }

        constexpr T getRight() const noexcept
        {
            return left + width;
        }

        constexpr T getBottom() const noexcept
        {
            return top + height;
        }

        constexpr QuadVector2<T> getTopLeft() const noexcept
        {
            return QuadVector2<T>(left, top);
        }

        constexpr QuadVector2<T> getCenter() const noexcept
        {
            return QuadVector2<T>(left + width / 2, top + height / 2);
        }

        constexpr QuadVector2<T> getSize() const noexcept
        {
            return QuadVector2<T>(width, height);
        }

        constexpr bool contains(const QuadBox<T> &QuadBox) const noexcept
        {
            return left <= QuadBox.left && QuadBox.getRight() <= getRight() &&
                   top <= QuadBox.top && QuadBox.getBottom() <= getBottom();
        }

        constexpr bool intersects(const QuadBox<T> &QuadBox) const noexcept
        {
            return !(left >= QuadBox.getRight() || getRight() <= QuadBox.left ||
                     top >= QuadBox.getBottom() || getBottom() <= QuadBox.top);
        }
    };

}