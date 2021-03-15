#ifndef __POINT_H_
#define __POINT_H_

#include <iostream>
#include <vector>
#include <memory>
#include <limits>

#include "Vector2.h"

class Point{
public:
	Vector2<int> m_Coordinate;
	std::weak_ptr<Point> m_Parent;

private:
	float f_cost;
	float g_cost;
	float h_cost;

public:
	Point(Vector2<int> point){
		this->m_Coordinate  = point;
		this->f_cost = 0.0f;
		this->g_cost = 0.0f;
		this->h_cost = 0.0f;
	}

	Point(){
		this->m_Coordinate  = Vector2<int>(0,0);
		this->f_cost = 0.0f;
		this->g_cost = 0.0f;
		this->h_cost = 0.0f;
	}

	Point(Point&& other) = default;
	Point(const Point& other) = default;
	Point& operator=(Point&& other) = default; 
	Point& operator=(const Point& other) = default; 
	~Point() = default;
	

	std::shared_ptr<Point> Copy(){
		std::shared_ptr<Point> copy_ptr = std::make_shared<Point>(this->getCoord());

		copy_ptr->set_f_cost(this->get_f_cost());
		copy_ptr->set_g_cost(this->get_g_cost());
		copy_ptr->set_h_cost(this->get_h_cost());

		copy_ptr->m_Parent = this->m_Parent.lock();

		return copy_ptr;
	}

	void calc_f_cost() { this->f_cost = this->g_cost + this->h_cost; }

	float get_h_cost(){ return this->h_cost; }
	float get_g_cost(){ return this->g_cost; }
	float get_f_cost() const { return this->f_cost; }

	Vector2<int> getCoord(){ return this->m_Coordinate; }

	void set_f_cost(float f_cost){ this->f_cost = f_cost; }
	void set_g_cost(float g_cost){ this->g_cost = g_cost; }
	void set_h_cost(float h_cost){ this->h_cost = h_cost; }
	void setCoord(Vector2<int> newCoord){ this->m_Coordinate = newCoord; }

	bool operator==(const Point& rhs) const{
		if (this->m_Coordinate.x == rhs.m_Coordinate.x && this->m_Coordinate.y == rhs.m_Coordinate.y)
			return true;
		return false;
	}

	bool operator==(std::shared_ptr<Point>& rhs) const{
		if (this->m_Coordinate.x == rhs->m_Coordinate.x && this->m_Coordinate.y == rhs->m_Coordinate.y)
			return true;
		return false;
	}

	
	friend bool operator>(const std::shared_ptr<Point> &lhs, const std::shared_ptr<Point> &rhs) {
            return lhs->get_f_cost() > rhs->get_f_cost();
	}
};

class ComparePointPQ
{
public:
	bool operator() (const std::shared_ptr<Point> first, const std::shared_ptr<Point> second){
		return first->get_f_cost() > second->get_f_cost();
	}
};

class ComparePointNoPTRPQ
{
public:
	bool operator() (const Point& first, const Point& second){
		return first.get_f_cost() > second.get_f_cost();
	}
};

class ComparePointSET{
public:
	bool operator() (std::shared_ptr<Point> first, std::shared_ptr<Point> second)
    {
		return first->m_Coordinate == second->m_Coordinate;
		
	}
};

class ComparePointNoPTRSET{
public:
	bool operator() (Point& first, Point& second)
    {
		return first.m_Coordinate == second.m_Coordinate;
		
	}
};


class PointHasherNoPTR{
public:
	size_t operator() (const Point& obj) const
    {
		return std::hash<int>()(obj.m_Coordinate.x) ^ std::hash<int>()(obj.m_Coordinate.x);
	}
};

#endif
