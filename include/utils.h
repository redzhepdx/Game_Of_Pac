#ifndef __UTILS_H_
#define __UTILS_H_

#include <iostream>
#include <set>
#include <vector>
#include <queue>
#include <memory>

#include "Vector2.h"
#include "point.h"
#include "Maze.h"

#define WIDTH 800
#define HEIGHT 800
#define DELAY_NUMBER 10000000
#define Updates_Per_Second 60

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

//TODO : FEELS LIKE NOT WELL OPTIMIZED FOR MEMORY -> WILL TEST THE MEMORY(Shared Memory Restricted)
static inline std::vector<Vector2<int>> findRoute(std::vector<std::vector<TileType>> matrix,
                                                  std::vector<std::vector<std::shared_ptr<Point>>> grid,
                                                  Vector2<int> start, Vector2<int> end){

    std::vector<Vector2<int>> route;
    //A* Algorithm
    //SET FOR THE CHECK
    std::set<std::shared_ptr<Point>> closedSet; 
    //CHECK SET FOR CLOSED POINTS
    std::set<std::shared_ptr<Point>> openSetContains;
    //PRIORTY QUEUE FOR THE NEXT STEPS
    std::priority_queue<std::shared_ptr<Point>, std::vector<std::shared_ptr<Point>>, ComparePointPQ> openSet;

    std::shared_ptr<Point> startVertex = std::make_shared<Point>(start);
    std::shared_ptr<Point> destVertex = std::make_shared<Point>(end);

    openSet.push(startVertex); //Push First Point

    int max_row = matrix.size() - 1;
    int max_col = matrix[0].size() - 1;

    while(!openSet.empty()){
        std::shared_ptr<Point> currentVertex = openSet.top(); //It moves first
        openSet.pop(); // Delete it from PQueue
        closedSet.insert(currentVertex); //Add to Check Set
        openSetContains.insert(currentVertex); //Mark it as it is in the openSet

        //Retrace Route
        if(currentVertex->coord == destVertex->coord){
            while(currentVertex->coord != startVertex->coord){
                route.push_back(Vector2<int>(currentVertex->coord.x, currentVertex->coord.y));
                currentVertex = grid[currentVertex->parent->coord.x][currentVertex->parent->coord.y];
            }
            return route;
        }

        //Look Neigbours
        std::vector<std::shared_ptr<Point>> neighbours;

        std::shared_ptr<Point> ng1 = grid[currentVertex->coord.x - 1][ currentVertex->coord.y];
        if (closedSet.find(ng1) == closedSet.end() && matrix[ng1->coord.x][ng1->coord.y] == Empty && isInMatrix<int>(0, 0, max_row, max_col, ng1->coord)){
            neighbours.push_back(ng1);
        }
            
        std::shared_ptr<Point> ng2 = grid[currentVertex->coord.x][ currentVertex->coord.y - 1];
        if (closedSet.find(ng2) == closedSet.end() && matrix[ng2->coord.x][ng2->coord.y] == Empty && isInMatrix<int>(0, 0, max_row, max_col, ng2->coord)){
            neighbours.push_back(ng2);
        }
            
        std::shared_ptr<Point> ng3 = grid[currentVertex->coord.x + 1][ currentVertex->coord.y];
        if (closedSet.find(ng3) == closedSet.end() && matrix[ng3->coord.x][ng3->coord.y] == Empty && isInMatrix<int>(0, 0, max_row, max_col, ng3->coord)){
            neighbours.push_back(ng3);
        }
            
        std::shared_ptr<Point> ng4 = grid[currentVertex->coord.x][ currentVertex->coord.y + 1];
        if (closedSet.find(ng4) == closedSet.end() && matrix[ng4->coord.x][ng4->coord.y] == Empty && isInMatrix<int>(0, 0, max_row, max_col, ng4->coord)){
            neighbours.push_back(ng4);
        }
            
        for(auto& ng : neighbours){
            float newCostToNeighbour = currentVertex->get_g_cost() + dist_euc<int>(currentVertex->coord, ng->coord);
            if(newCostToNeighbour < ng->get_g_cost() || (openSetContains.find(ng) == openSetContains.end())){
                grid[ng->coord.x][ng->coord.y]->set_g_cost(newCostToNeighbour);
                grid[ng->coord.x][ng->coord.y]->set_h_cost(dist_euc<int>(ng->coord, destVertex->coord));
                grid[ng->coord.x][ng->coord.y]->calc_f_cost();
                grid[ng->coord.x][ng->coord.y]->parent = grid[currentVertex->coord.x][currentVertex->coord.y];

                if((openSetContains.find(ng) == openSetContains.end())){
                   openSet.push(grid[ng->coord.x][ng->coord.y]); // Add to PQ
                   openSetContains.insert(ng); // Mark It
                }
            }
        }
    }
    return route; //WILL LOOK THIS LATER
}

#endif
