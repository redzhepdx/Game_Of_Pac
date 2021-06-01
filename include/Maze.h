#ifndef _MAZE_H_
#define _MAZE_H_

#include <iostream>
#include <unordered_map>
#include <memory>
#include <vector>

#include "Area.h"
#include "Vector2.h"
#include "config.h"

class Maze
{
public:
	int32_t m_Width;
	int32_t m_Height;
	std::vector<std::vector<TileType>> m_Matrix;

public:
	TileType pos2Tile(Vector2<float> pos);
	Vector2<int32_t> pos2MtrCoord(Vector2<float> pos);
	Maze(int32_t width, int32_t height);
	Maze();
	~Maze();
	std::unique_ptr<Maze> Copy();
	void print();
};

#endif
