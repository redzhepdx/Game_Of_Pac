#include "Maze.h"
#include <vector>
#include <cmath>

Maze::Maze(int32_t width, int32_t height) : m_Width(width), m_Height(height)
{
	for (int32_t i = 0; i < height + 1; ++i)
	{
		std::vector<TileType> inner_vec;
		for (int32_t j = 0; j < width + 1; ++j)
		{
			if (i == 0 || j == width)
			{
				inner_vec.push_back(Wall);
			}
			else
			{
				inner_vec.push_back(Wall);
			}
		}
		m_Matrix.push_back(inner_vec);
	}
}

Maze::Maze()
{
}

Maze::~Maze()
{
	m_Matrix.clear();
}

TileType Maze::pos2Tile(Vector2<float> pos)
{
	int32_t row_pos = m_Height - static_cast<int32_t>(std::floor(pos.y / SQUARE_SIZE));
	int32_t col_pos = static_cast<int32_t>(std::floor(pos.x / SQUARE_SIZE));
	return m_Matrix[row_pos % m_Matrix.size()][col_pos % m_Matrix.size()];
}

Vector2<int32_t> Maze::pos2MtrCoord(Vector2<float> pos)
{
	int32_t row_pos = m_Height - static_cast<int32_t>(std::floor(pos.y / SQUARE_SIZE));
	int32_t col_pos = static_cast<int32_t>(std::floor(pos.x / SQUARE_SIZE));
	return Vector2<int32_t>(row_pos, col_pos);
}

void Maze::print()
{
	for (int32_t row = 0; row < m_Width + 1; ++row)
	{
		for (int32_t col = 0; col < m_Height + 1; ++col)
		{
			std::cout << m_Matrix[row][col] << " ";
		}
		std::cout << std::endl;
	}
}

std::unique_ptr<Maze> Maze::Copy()
{
	std::unique_ptr<Maze> copy = std::make_unique<Maze>();

	copy->m_Width = m_Width;
	copy->m_Height = m_Height;
	copy->m_Matrix = m_Matrix;

	return std::move(copy);
}
