#ifndef __VECTOR2D__
#define __VECTOR2D__

#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <memory>

template <typename T>
class Vector2{
public:
	T x;
	T y;
public:
	Vector2();
	Vector2(T x, T y);
	Vector2(const Vector2<T> &other);

	Vector2<T> operator+(const Vector2<T> &other) const;
	Vector2<T> & operator=(const Vector2<T> & other);
	void operator+=(const Vector2<T> &other);
	void operator-=(const Vector2<T> &other);
	bool operator==(const Vector2<T> &other) const;
	bool operator!=(const Vector2<T> &other) const;
};

#endif
