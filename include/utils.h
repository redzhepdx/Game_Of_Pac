#ifndef __UTILS_H_
#define __UTILS_H_

#include <iostream>
#include <unordered_set>
#include <vector>
#include <queue>
#include <memory>
#include <cmath>
#include <cstdint>

#include <torch/torch.h>

#include "Vector2.h"
#include "point.h"
#include "Maze.h"
#include "config.h"

typedef std::vector<std::vector<std::shared_ptr<Point>>> shared_point_matrix;

namespace utility{

static inline unsigned char * readImage(const char *filename, int width, int height){
    unsigned char *imageData;
    FILE *file;

    file = fopen(filename, "r");
    if (file == NULL) return 0;

    imageData = (unsigned char *) malloc(width * height * 4);
    fread(imageData, width * height * 4, 1, file);
    fclose(file);

    return imageData;
}

static inline void basic_delay(int slow_x){
    for(int i = 0; i < slow_x * DELAY_NUMBER; i++){}
}

static inline float angle2Rad(float angle){
    return angle * M_PI / 180.0;
}

template<typename T>
static inline float dist_euc(Vector2<T> vec_1, Vector2<T> vec_2){
    return std::sqrt((vec_1.x - vec_2.x) * (vec_1.x - vec_2.x) + (vec_1.y - vec_2.y) * (vec_1.y - vec_2.y));
}

template<typename T>
static inline bool isInMatrix(T min_col, T min_row, T max_col, T max_row, Vector2<T> coord){
    return coord.x >= min_col && coord.x <= max_col && coord.y >= min_row && coord.y <= max_row;
}

static inline std::vector<Vector2<int>> findRoute(tile_type_matrix matrix,
                                                  shared_point_matrix grid,
                                                  Vector2<int> start, Vector2<int> end){
    std::vector<Vector2<int>> route;
    //A* Algorithm
    Vector2<int> destination = end;

    // Offset based player tile position correction
    if(matrix[destination.x][destination.y] == Wall){
        if(destination.x - 1 > 0){
            if(matrix[destination.x - 1][destination.y] != Wall){
                destination.x -= 1;
            }
            else if(destination.y + 1 < matrix[0].size()){
                if(matrix[destination.x - 1][destination.y + 1] != Wall){
                    destination.x -= 1;
                    destination.y += 1;
                }
                else if(matrix[destination.x][destination.y + 1] != Wall){
                    destination.y += 1;
                }
            }
        }
        else{
            if(destination.y + 1 < matrix[0].size()){
                if(matrix[destination.x][destination.y + 1] != Wall){
                    destination.y += 1;
                }
            }
        }
    }

    //SET FOR THE CHECK
    std::unordered_set<std::shared_ptr<Point>> closedSet; 
    
    //CHECK SET FOR CLOSED POINTS
    std::unordered_set<std::shared_ptr<Point>> openSetContains;

    //PRIORTY QUEUE FOR THE NEXT STEPS
    std::priority_queue<std::shared_ptr<Point>, std::vector<std::shared_ptr<Point>>, ComparePointPQ> openSet;

    std::shared_ptr<Point> startVertex = std::make_shared<Point>(start);
    std::shared_ptr<Point> destVertex = std::make_shared<Point>(destination);

    openSet.push(startVertex); //Push First Point

    int max_row = matrix.size() - 1;
    int max_col = matrix[0].size() - 1;

    while(!openSet.empty()){
        std::shared_ptr<Point> currentVertex = openSet.top(); //It moves first
        openSet.pop(); // Delete it from PQueue
        closedSet.insert(currentVertex); //Add to Check Set
        openSetContains.insert(currentVertex); //Mark it as it is in the openSet

        //Retrace Route
        if(currentVertex->m_Coordinate == destVertex->m_Coordinate){
            while(currentVertex->m_Coordinate != startVertex->m_Coordinate){
                route.push_back(Vector2<int>(currentVertex->m_Coordinate.x, currentVertex->m_Coordinate.y));
                currentVertex = grid[currentVertex->m_Parent.lock()->m_Coordinate.x][currentVertex->m_Parent.lock()->m_Coordinate.y];
            }
            return route;
        }

        //Look Neigbours
        std::vector<std::shared_ptr<Point>> neighbours;

        if( isInMatrix<int>(0, 0, max_row, max_col, Vector2<int>(currentVertex->m_Coordinate.x - 1, currentVertex->m_Coordinate.y))){
            std::shared_ptr<Point> ng1 = grid[currentVertex->m_Coordinate.x - 1][ currentVertex->m_Coordinate.y];
            if (closedSet.find(ng1) == closedSet.end() && 
                (matrix[ng1->m_Coordinate.x][ng1->m_Coordinate.y] == Empty || matrix[ng1->m_Coordinate.x][ng1->m_Coordinate.y] == Teleport)){
                neighbours.push_back(ng1);
            }
        }

        if( isInMatrix<int>(0, 0, max_row, max_col, Vector2<int>(currentVertex->m_Coordinate.x, currentVertex->m_Coordinate.y - 1))){
            std::shared_ptr<Point> ng2 = grid[currentVertex->m_Coordinate.x][ currentVertex->m_Coordinate.y - 1];
            if (closedSet.find(ng2) == closedSet.end() && 
                (matrix[ng2->m_Coordinate.x][ng2->m_Coordinate.y] == Empty || matrix[ng2->m_Coordinate.x][ng2->m_Coordinate.y] == Teleport)){
                neighbours.push_back(ng2);
            }
        }
        
        if (isInMatrix<int>(0, 0, max_row, max_col, Vector2<int>(currentVertex->m_Coordinate.x + 1, currentVertex->m_Coordinate.y))){
            std::shared_ptr<Point> ng3 = grid[currentVertex->m_Coordinate.x + 1][ currentVertex->m_Coordinate.y];
            if (closedSet.find(ng3) == closedSet.end() && 
                (matrix[ng3->m_Coordinate.x][ng3->m_Coordinate.y] == Empty || matrix[ng3->m_Coordinate.x][ng3->m_Coordinate.y] == Teleport)){
                neighbours.push_back(ng3);
            }
        }
        
        if(isInMatrix<int>(0, 0, max_row, max_col, Vector2<int>(currentVertex->m_Coordinate.x, currentVertex->m_Coordinate.y + 1))){
            std::shared_ptr<Point> ng4 = grid[currentVertex->m_Coordinate.x][ currentVertex->m_Coordinate.y + 1];
            if (closedSet.find(ng4) == closedSet.end() &&
                (matrix[ng4->m_Coordinate.x][ng4->m_Coordinate.y] == Empty || matrix[ng4->m_Coordinate.x][ng4->m_Coordinate.y] == Teleport)){
                neighbours.push_back(ng4);
            }
        }
        

        for(auto& ng : neighbours){
            float newCostToNeighbour = currentVertex->get_g_cost() + utility::dist_euc<int>(currentVertex->m_Coordinate, ng->m_Coordinate);
            if(newCostToNeighbour < ng->get_g_cost() || (openSetContains.find(ng) == openSetContains.end())){
                grid[ng->m_Coordinate.x][ng->m_Coordinate.y]->set_g_cost(newCostToNeighbour);
                grid[ng->m_Coordinate.x][ng->m_Coordinate.y]->set_h_cost(utility::dist_euc<int>(ng->m_Coordinate, destVertex->m_Coordinate));
                grid[ng->m_Coordinate.x][ng->m_Coordinate.y]->calc_f_cost();
                grid[ng->m_Coordinate.x][ng->m_Coordinate.y]->m_Parent = grid[currentVertex->m_Coordinate.x][currentVertex->m_Coordinate.y];

                if((openSetContains.find(ng) == openSetContains.end())){
                openSet.push(grid[ng->m_Coordinate.x][ng->m_Coordinate.y]); // Add to PQ
                openSetContains.insert(ng); // Mark It
                }
            }
        }
    }
    return route;
}

template <typename T>
static inline void swap(T& x, T& y){
    T temp = x;
    x = y;
    y = temp;
}

static inline bool rayCastObstacleCheck(tile_type_matrix matrix, 
                                        Vector2<int> currentPos, Vector2<int> targetPos){
    int x0 = currentPos.y;
    int y0 = currentPos.x;

    int x1 = targetPos.y;
    int y1 = targetPos.x;

    // RowCurrent -
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    
    if(steep){
        swap<int>(x0, y0);
        swap<int>(x1, y1);
    }

    if(x0 > x1){
        swap<int>(x0, x1);
        swap<int>(y0, y1);
    }

    int deltax = x1 - x0;
    int deltay = std::abs(y1 - y0);
    int error = 0;

    int ystep;
    int y = y0;

    if(y0 < y1){
        ystep = 1;
    }
    else{
        ystep = -1;
    }

    for(int x = x0; x <= x1; ++x){
        if(steep){
            // Check tile y, x
            if(matrix[x][y] == Wall){
                return false;
            }
        }
        else{
            // Check tile x, y
            if(matrix[y][x] == Wall){
                return false;
            }
        }

        error += deltay;
        
        if(2 * error >= deltax){
            y += ystep;
            error -= deltax;
        }
    }

    return true;
}

}
#endif
