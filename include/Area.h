#ifndef __AREA__
#define __AREA__
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <memory>

typedef struct Bbox
{
public:
	float bottom;
	float top;
	float left;
	float right;
} Bbox, *pBox;

class Area
{

public:
	std::unique_ptr<Bbox> m_Box;

public:
	Area();
	Area(float bottom, float top, float left, float right);
	~Area();
	std::unique_ptr<Area> Copy();
};

#endif
