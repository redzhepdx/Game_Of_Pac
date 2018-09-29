#include "Vector2.h"

template <typename T>
Vector2<T>::Vector2(){ }
template Vector2<float>::Vector2();
template Vector2<int>::Vector2();

template <typename T>
Vector2<T>::Vector2(T x, T y):x(x), y(y){ }
template Vector2<float>::Vector2(float x, float y);
template Vector2<int>::Vector2(int x, int y);


template <typename T>
Vector2<T>::Vector2(const Vector2<T> &other){
	this->x = other.x;
	this->y = other.y;
}
template Vector2<float>::Vector2(const Vector2<float> &other);
template Vector2<int>::Vector2(const Vector2<int> &other);

template <typename T>
Vector2<T> Vector2<T>::operator+(const Vector2<T> &other) const{
	Vector2<T> new_vector(0, 0);
	new_vector.x = this->x + other.x;
	new_vector.y = this->y + other.y;
	return new_vector;
}
template Vector2<float> Vector2<float>::operator+(const Vector2<float> &other) const;
template Vector2<int> Vector2<int>::operator+(const Vector2<int> &other) const;

template <typename T>
Vector2<T> & Vector2<T>::operator=(const Vector2<T> &other){
	this->x = other.x;
	this->y = other.y;
	return *this;
}
template Vector2<float> & Vector2<float>::operator=(const Vector2<float> &other);
template Vector2<int> & Vector2<int>::operator=(const Vector2<int> &other);

template <typename T>
void Vector2<T>::operator+=(const Vector2<T> &other){
	this->x += other.x;
	this->y += other.y;
}
template void Vector2<float>::operator+=(const Vector2<float> &other);
template void Vector2<int>::operator+=(const Vector2<int> &other);

template <typename T>
void Vector2<T>::operator-=(const Vector2<T> &other){
	this->x -= other.x;
	this->y -= other.y;
}
template void Vector2<float>::operator-=(const Vector2<float> &other);
template void Vector2<int>::operator-=(const Vector2<int> &other);

template <typename T>
bool Vector2<T>::operator==(const Vector2<T> &other) const{
	if (this->x == other.x && this->y == other.y)
		return true;
	return false;
}
template bool Vector2<float>::operator==(const Vector2<float> &other) const;
template bool Vector2<int>::operator==(const Vector2<int> &other) const;

template <typename T>
bool Vector2<T>::operator!=(const Vector2<T> &other) const{
	if (this->x != other.x || this->y != other.y)
		return true;
	return false;
}
template bool Vector2<float>::operator!=(const Vector2<float> &other) const;
template bool Vector2<int>::operator!=(const Vector2<int> &other) const;