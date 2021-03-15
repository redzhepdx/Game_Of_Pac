#include "Area.h"
Area::Area(){
	this->m_Box = std::make_unique<Bbox>();
}

Area::Area(float bottom, float top, float left, float right){
	this->m_Box         = std::make_unique<Bbox>();
	this->m_Box->bottom = bottom;
	this->m_Box->top    = top;
	this->m_Box->left   = left;
	this->m_Box->right  = right;
}

Area::~Area(){
	//free(this->Box);
	//delete this->Box;
}

std::unique_ptr<Area> Area::Copy(){
	std::unique_ptr<Area> copy = std::make_unique<Area>();
	copy->m_Box->bottom        = this->m_Box->bottom;
	copy->m_Box->top           = this->m_Box->top;
	copy->m_Box->left          = this->m_Box->left;
	copy->m_Box->right         = this->m_Box->right;
	return std::move(copy);
}
