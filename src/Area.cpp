#include "Area.h"
Area::Area(){
	this->m_Box = (pBox)malloc(sizeof(Bbox));
}

Area::Area(float bottom, float top, float left, float right){
	this->m_Box = (pBox)malloc(sizeof(Bbox));
	this->m_Box->bottom = bottom;
	this->m_Box->top = top;
	this->m_Box->left = left;
	this->m_Box->right = right;
}

Area::~Area(){
	//free(this->Box);
	//delete this->Box;
}

std::unique_ptr<Area> Area::Copy(){
	std::unique_ptr<Area> copy = std::make_unique<Area>();
	copy->m_Box = this->m_Box;
	return copy;
}
