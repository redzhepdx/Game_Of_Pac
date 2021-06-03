#ifndef __POINT_H_
#define __POINT_H_

#include <iostream>
#include <vector>
#include <memory>
#include <limits>

#include "Vector2.h"

class Point {
public:
    Vector2<int32_t> m_Coordinate;
    std::weak_ptr<Point> m_Parent;

private:
    float f_cost;
    float g_cost;
    float h_cost;

public:
    explicit Point(const Vector2<int32_t> &point) {
        m_Coordinate = point;
        f_cost = 0.0f;
        g_cost = 0.0f;
        h_cost = 0.0f;
    }

    Point() {
        m_Coordinate = Vector2<int32_t>(0, 0);
        f_cost = 0.0f;
        g_cost = 0.0f;
        h_cost = 0.0f;
    }

    Point(Point &&other) = default;

    Point(const Point &other) = default;

    Point &operator=(Point &&other) = default;

    Point &operator=(const Point &other) = default;

    ~Point() = default;

    [[nodiscard]] std::shared_ptr<Point> Copy() const {
        std::shared_ptr<Point> copy_ptr = std::make_shared<Point>(getCoord());

        copy_ptr->set_f_cost(get_f_cost());
        copy_ptr->set_g_cost(get_g_cost());
        copy_ptr->set_h_cost(get_h_cost());

        copy_ptr->m_Parent = m_Parent.lock();

        return copy_ptr;
    }

    void calc_f_cost() { f_cost = g_cost + h_cost; }

    [[nodiscard]] float get_h_cost() const { return h_cost; }

    [[nodiscard]] float get_g_cost() const { return g_cost; }

    [[nodiscard]] float get_f_cost() const { return f_cost; }

    [[nodiscard]] Vector2<int32_t> getCoord() const { return m_Coordinate; }

    void set_f_cost(float new_f_cost) { f_cost = new_f_cost; }

    void set_g_cost(float new_g_cost) { this->g_cost = new_g_cost; }

    void set_h_cost(float new_h_cost) { this->h_cost = new_h_cost; }

    void setCoord(const Vector2<int32_t> &newCoord) { m_Coordinate = newCoord; }

    bool operator==(const Point &rhs) const {
        if (m_Coordinate.x == rhs.m_Coordinate.x && m_Coordinate.y == rhs.m_Coordinate.y)
            return true;
        return false;
    }

    bool operator==(std::shared_ptr<Point> &rhs) const {
        if (m_Coordinate.x == rhs->m_Coordinate.x && m_Coordinate.y == rhs->m_Coordinate.y)
            return true;
        return false;
    }

    friend bool operator>(const std::shared_ptr<Point> &lhs, const std::shared_ptr<Point> &rhs) {
        return lhs->get_f_cost() > rhs->get_f_cost();
    }
};

class ComparePointPQ {
public:
    bool operator()(const std::shared_ptr<Point> &first, const std::shared_ptr<Point> &second) {
        return first->get_f_cost() > second->get_f_cost();
    }
};

class ComparePointNoPTRPQ {
public:
    bool operator()(const Point &first, const Point &second) {
        return first.get_f_cost() > second.get_f_cost();
    }
};

class ComparePointSET {
public:
    bool operator()(const std::shared_ptr<Point> &first, const std::shared_ptr<Point> &second) {
        return first->m_Coordinate == second->m_Coordinate;
    }
};

class ComparePointNoPTRSET {
public:
    bool operator()(Point &first, Point &second) {
        return first.m_Coordinate == second.m_Coordinate;
    }
};

class PointHasherNoPTR {
public:
    size_t operator()(const Point &obj) const {
        return std::hash<int32_t>()(obj.m_Coordinate.x) ^ std::hash<int32_t>()(obj.m_Coordinate.x);
    }
};

#endif
