#ifndef __AREA__
#define __AREA__
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <memory>

typedef struct{
	float bottom;
	float top;
	float left;
	float right;
}Bbox, *pBox;

class Area{

public:
	pBox Box;

public:
	Area();
	Area(float bottom, float top, float left, float right);
	~Area();
	std::unique_ptr<Area> Copy();
};

#endif
