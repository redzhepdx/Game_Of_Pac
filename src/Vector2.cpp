#include "Vector2.h"

template<typename T>
Vector2<T>::Vector2() {}

template Vector2<float>::Vector2();

template Vector2<int32_t>::Vector2();

template<typename T>
Vector2<T>::Vector2(T x, T y) : x(x), y(y) {}

template Vector2<float>::Vector2(float x, float y);

template Vector2<int32_t>::Vector2(int x, int y);

template<typename T>
Vector2<T>::Vector2(const Vector2<T> &other) {
    this->x = other.x;
    this->y = other.y;
}

template Vector2<float>::Vector2(const Vector2<float> &other);

template Vector2<int32_t>::Vector2(const Vector2<int32_t> &other);

template<typename T>
T Vector2<T>::magnitute() const {
    return x * x + y * y;
}

template float Vector2<float>::magnitute() const;

template int Vector2<int32_t>::magnitute() const;

template<typename T>
Vector2<T> Vector2<T>::normalize() {
    T magnitute = this->magnitute();

    auto inv_sqrt_mag = inverse_sqrt<float, 2>((float) magnitute);

    this->x *= (T) inv_sqrt_mag;
    this->y *= (T) inv_sqrt_mag;

    return *this;
}

template Vector2<float> Vector2<float>::normalize();

template Vector2<int32_t> Vector2<int32_t>::normalize();

template<typename T>
float Vector2<T>::angle() const {
    return std::atan2((double) this->y, (double) this->x) / M_PI * 180.0;
}

template float Vector2<float>::angle() const;

template float Vector2<int32_t>::angle() const;

template<typename T>
Vector2<T> Vector2<T>::operator+(const Vector2<T> &other) const {
    Vector2<T> new_vector(0, 0);
    new_vector.x = this->x + other.x;
    new_vector.y = this->y + other.y;
    return new_vector;
}

template Vector2<float> Vector2<float>::operator+(const Vector2<float> &other) const;

template Vector2<int32_t> Vector2<int32_t>::operator+(const Vector2<int32_t> &other) const;

template<typename T>
Vector2<T> Vector2<T>::operator-(const Vector2<T> &other) const {
    Vector2<T> new_vector(0, 0);
    new_vector.x = this->x - other.x;
    new_vector.y = this->y - other.y;
    return new_vector;
}

template Vector2<float> Vector2<float>::operator-(const Vector2<float> &other) const;

template Vector2<int32_t> Vector2<int32_t>::operator-(const Vector2<int32_t> &other) const;

template<typename T>
Vector2<T> &Vector2<T>::operator=(const Vector2<T> &other) {
    this->x = other.x;
    this->y = other.y;
    return *this;
}

template Vector2<float> &Vector2<float>::operator=(const Vector2<float> &other);

template Vector2<int32_t> &Vector2<int32_t>::operator=(const Vector2<int32_t> &other);

template<typename T>
Vector2<T> Vector2<T>::operator*(const T value) const {
    return Vector2<T>((T) (this->x * value), (T) (this->y * value));
}

template Vector2<float> Vector2<float>::operator*(const float value) const;

template Vector2<int32_t> Vector2<int32_t>::operator*(const int value) const;

template<typename T>
void Vector2<T>::operator+=(const Vector2<T> &other) {
    this->x += other.x;
    this->y += other.y;
}

template void Vector2<float>::operator+=(const Vector2<float> &other);

template void Vector2<int32_t>::operator+=(const Vector2<int32_t> &other);

template<typename T>
void Vector2<T>::operator-=(const Vector2<T> &other) {
    this->x -= other.x;
    this->y -= other.y;
}

template void Vector2<float>::operator-=(const Vector2<float> &other);

template void Vector2<int32_t>::operator-=(const Vector2<int32_t> &other);

template<typename T>
bool Vector2<T>::operator==(const Vector2<T> &other) const {
    if (this->x == other.x && this->y == other.y)
        return true;
    return false;
}

template bool Vector2<float>::operator==(const Vector2<float> &other) const;

template bool Vector2<int32_t>::operator==(const Vector2<int32_t> &other) const;

template<typename T>
bool Vector2<T>::operator!=(const Vector2<T> &other) const {
    if (this->x != other.x || this->y != other.y)
        return true;
    return false;
}

template bool Vector2<float>::operator!=(const Vector2<float> &other) const;

template bool Vector2<int32_t>::operator!=(const Vector2<int32_t> &other) const;