#include "Area.h"

Area::Area() {
    m_Box = std::make_unique<Bbox>();
}

Area::Area(float bottom, float top, float left, float right) {
    m_Box = std::make_unique<Bbox>();
    m_Box->bottom = bottom;
    m_Box->top = top;
    m_Box->left = left;
    m_Box->right = right;
}

std::unique_ptr<Area> Area::Copy() const {
    std::unique_ptr<Area> copy = std::make_unique<Area>();
    copy->m_Box->bottom = m_Box->bottom;
    copy->m_Box->top = m_Box->top;
    copy->m_Box->left = m_Box->left;
    copy->m_Box->right = m_Box->right;
    return copy;
}
