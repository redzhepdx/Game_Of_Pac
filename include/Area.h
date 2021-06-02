#ifndef __AREA__
#define __AREA__

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <memory>
#include "spdlog/spdlog.h"

typedef struct Bbox {
public:
    float bottom;
    float top;
    float left;
    float right;
} Bbox, *pBox;

class Area {

public:
    std::unique_ptr<Bbox> m_Box;

public:
    Area();

    Area(float bottom, float top, float left, float right);

    ~Area() = default;

    [[nodiscard]] std::unique_ptr<Area> Copy() const;

    void print() const;
};

#endif
