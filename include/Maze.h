#ifndef _MAZE_H_
#define _MAZE_H_

#include <iostream>
#include <unordered_map>
#include <memory>

#include "Area.h"
#include "Vector2.h"

#define Square_Size 20

enum TileType {Empty, Wall, Obstacle, Teleport};

class Maze{
public:
	int width;
	int height;
	std::vector<std::vector<TileType>> matrix;

public:
	TileType pos2Tile(Vector2<float> pos);
	Vector2<int> pos2MtrCoord(Vector2<float> pos);
	Maze(int width, int height);
	Maze();
	~Maze();
	std::unique_ptr<Maze> Copy();
	void print();
};

#endif
