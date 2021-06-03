#include "Maze.h"
#include <vector>
#include <cmath>

Maze::Maze(int32_t width, int32_t height) : m_Width(width), m_Height(height) {
    m_Matrix = std::vector<std::vector<TileType>>(height + 1, std::vector<TileType>(width, Wall));
}

Maze::~Maze() {
    m_Matrix.clear();
}

TileType Maze::pos2Tile(const Vector2<float> &pos) {
    auto row_pos = m_Height - static_cast<int32_t>(std::floor(pos.y / SQUARE_SIZE));
    auto col_pos = static_cast<int32_t>(std::floor(pos.x / SQUARE_SIZE));
    return m_Matrix[row_pos % m_Matrix.size()][col_pos % m_Matrix.size()];
}

Vector2<int32_t> Maze::pos2MtrCoord(const Vector2<float> &pos) const {
    auto row_pos = m_Height - static_cast<int32_t>(std::floor(pos.y / SQUARE_SIZE));
    auto col_pos = static_cast<int32_t>(std::floor(pos.x / SQUARE_SIZE));
    return Vector2<int32_t>(row_pos, col_pos);
}

void Maze::print() const {
    spdlog::info("Maze!");
    for (int32_t row = 0; row < m_Width + 1; ++row) {
        for (int32_t col = 0; col < m_Height + 1; ++col) {
            std::cout << m_Matrix[row][col] << " ";
        }
        std::cout << "\n";
    }
}

std::unique_ptr<Maze> Maze::Copy() const {
    std::unique_ptr<Maze> copy = std::make_unique<Maze>();

    copy->m_Width = m_Width;
    copy->m_Height = m_Height;
    copy->m_Matrix = m_Matrix;

    return copy;
}
