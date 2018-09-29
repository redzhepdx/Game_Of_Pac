#include "Area.h"
Area::Area(){
	this->Box = (pBox)malloc(sizeof(Bbox));
}

Area::Area(float bottom, float top, float left, float right){
	this->Box = (pBox)malloc(sizeof(Bbox));
	this->Box->bottom = bottom;
	this->Box->top = top;
	this->Box->left = left;
	this->Box->right = right;
}

Area::~Area(){
	//free(this->Box);
	//delete this->Box;
}

std::unique_ptr<Area> Area::Copy(){
	std::unique_ptr<Area> copy = std::make_unique<Area>();
	copy->Box = this->Box;
	return copy;
}
