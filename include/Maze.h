#ifndef _MAZE_H_
#define _MAZE_H_

#include <iostream>
#include <unordered_map>
#include <memory>
#include <vector>

#include "Area.h"
#include "Vector2.h"
#include "config.h"

class Maze{
public:
	int m_Width;
	int m_Height;
	std::vector<std::vector<TileType>> m_Matrix;

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
