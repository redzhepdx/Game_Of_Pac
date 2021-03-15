#include "Maze.h"
#include <vector>
#include <cmath>

Maze::Maze(int width, int height):m_Width(width), m_Height(height){
	for(int i = 0; i < height + 1; ++i){
		std::vector<TileType> inner_vec;
		for(int j = 0; j < width + 1; ++j){
			if(i == 0 || j == width){
				inner_vec.push_back(Wall);
			}
			else{
				inner_vec.push_back(Wall);
			}
		}
		this->m_Matrix.push_back(inner_vec);
	}
}

Maze::Maze(){

}

Maze::~Maze(){
	this->m_Matrix.clear();
}

TileType Maze::pos2Tile(Vector2<float> pos){
	int row_pos = this->m_Height - static_cast<int>(std::floor(pos.y / SQUARE_SIZE));
	int col_pos = static_cast<int>(std::floor(pos.x / SQUARE_SIZE));
	return this->m_Matrix[row_pos % this->m_Matrix.size()][col_pos % this->m_Matrix.size()];
}

Vector2<int> Maze::pos2MtrCoord(Vector2<float> pos){
	int row_pos = this->m_Height - static_cast<int>(std::floor(pos.y / SQUARE_SIZE));
	int col_pos = static_cast<int>(std::floor(pos.x / SQUARE_SIZE));
	return Vector2<int>(row_pos, col_pos);
}

void Maze::print(){
	for(int row = 0; row < this->m_Width + 1; ++row){
        for(int col = 0; col < this->m_Height + 1; ++col){
            std::cout << this->m_Matrix[row][col] << " ";
        }
        std::cout << std::endl;
    }
}

std::unique_ptr<Maze> Maze::Copy(){
	std::unique_ptr<Maze> copy = std::make_unique<Maze>();

	copy->m_Width  = this->m_Width;
	copy->m_Height = this->m_Height;
	copy->m_Matrix = this->m_Matrix;

	return std::move(copy);
}
