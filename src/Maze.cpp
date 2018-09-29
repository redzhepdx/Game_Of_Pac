#include "Maze.h"

Maze::Maze(int width, int height):width(width), height(height){
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
		this->matrix.push_back(inner_vec);
	}
}

Maze::Maze(){

}

Maze::~Maze(){
	this->matrix.clear();
}

TileType Maze::pos2Tile(Vector2<float> pos){
	int row_pos = this->height - static_cast<int>(std::floor(pos.y / Square_Size));
	int col_pos = static_cast<int>(std::floor(pos.x / Square_Size));
	return this->matrix[row_pos % this->matrix.size()][col_pos % this->matrix.size()];
}

Vector2<int> Maze::pos2MtrCoord(Vector2<float> pos){
	int row_pos = this->height - static_cast<int>(std::floor(pos.y / Square_Size));
	int col_pos = static_cast<int>(std::floor(pos.x / Square_Size));
	return Vector2<int>(row_pos, col_pos);
}

void Maze::print(){
	for(int row = 0; row < this->width + 1; ++row){
        for(int col = 0; col < this->height + 1; ++col){
            std::cout << this->matrix[row][col] << " ";
        }
        std::cout << std::endl;
    }
}

std::unique_ptr<Maze> Maze::Copy(){
	std::unique_ptr<Maze> copy = std::make_unique<Maze>();

	copy->width  = this->width;
	copy->height = this->height;
	copy->matrix = this->matrix;

	return copy;
}
