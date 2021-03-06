#include <nav_msgs/MapMetaData.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "astar.h"

//Cell Comparison class

bool cellComparison::operator() (const CellInfo& lhs, const CellInfo& rhs)
{
    return rhs.h < lhs.h;
}

//A* class

astar::astar()
{
    this->mapset = false;
    this->minHeap = new std::priority_queue<CellInfo, std::vector<CellInfo>, cellComparison>();
}

bool astar::haveMap()
{
    return this->mapset;
}

void astar::setMapMetadata(const nav_msgs::OccupancyGrid& map)
{
    this->mapset = true;
    this->mapResolution = map.info.resolution;
    this->mapHeight = map.info.height;
    this->mapWidth = map.info.width;
}

std::vector<int>* astar::getRute(const nav_msgs::OccupancyGrid& actualMap, int start, int goal)
{
    std::vector<int> *resultPath = new std::vector<int>;

    //Cells already evaluated
    std::vector<CellInfo> closedSet;

    auto startCell = createCell(start, goal);
    this->minHeap->push(startCell);
    closedSet.push_back(startCell);

    CellInfo actualCell;
    while(this->minHeap->size() > 0)
    {
        //Select cell to expand and push to the path
        actualCell = this->minHeap->top();

        std::cout << "Celda seleccionada" << actualCell.index << std::endl;

        this->minHeap->pop();
        resultPath->push_back(actualCell.index);
        if(actualCell.index == goal)
                return resultPath;
        //Expand neighbors
        for(int i = -4; i <= 4; i++)
        {
            int cellIndex = actualCell.index + i;
            bool cellAlreadyCheked = cellIsInVector(closedSet, cellIndex);
            bool cellIsFree = actualMap.data[cellIndex] == 0;
            if(!cellAlreadyCheked && cellIsFree)
            {
                auto newCell = createCell(cellIndex, goal);

                std::cout << "A expandir" << newCell.index << std::endl;

                this->minHeap->push(newCell);
                closedSet.push_back(newCell);
            }
        }
    }
    return resultPath;
}

CellInfo& astar::createCell(int value, int goal)
{
    CellInfo newCell;
    newCell.index = value;
    newCell.h = getDistance(value, goal);
    CellInfo &cellRef = newCell;
    return cellRef;
}

float astar::getDistance(int value, int goal)
{
    float m = this->mapResolution / 2;

    //Ubicacion de la celda actual
    float value_i = value % this->mapWidth;
    float value_j = value / this->mapWidth;
    float value_i_m = (float)value_i * this->mapResolution + m;
    float value_j_m = (float)value_j * this->mapResolution + m;

    //Ubicacion de la celda objetivo
    int goal_i = goal % this->mapWidth;
    int goal_j = goal / this->mapWidth;
    float goal_i_m = (float)goal_i * this->mapResolution + m;
    float goal_j_m = (float)goal_j * this->mapResolution + m;

    return sqrt(pow(value_i - goal_i_m, 2) + pow(value_j - goal_j_m, 2));
}

bool astar::cellIsInVector(std::vector<CellInfo> &cellVector, int cellIndex)
{
    for(auto i = cellVector.begin(); i != cellVector.end(); i++)
    {
        auto actualCell = *i;
        if(actualCell.index == cellIndex) return true;
    }
    return false;
}

bool astar::cellIsInVector(std::vector<CellInfo> &cellVector, const CellInfo &cell)
{
    for(auto i = cellVector.begin(); i != cellVector.end(); i++)
    {
        auto actualCell = *i;
        if(actualCell.index == cell.index) return true;
    }
    return false;
}

//Internal testing method

void astar::Test()
{
    std::cout << this->mapResolution << "," << this->mapHeight << "," << this->mapWidth << std::endl;
    
    CellInfo c1;
    c1.index = 15;

    CellInfo c2;
    c2.index = 5;

    CellInfo c3;
    c3.index = 10;

    CellInfo c4;
    c3.index = 2;

    std::vector<CellInfo> vect;
    vect.push_back(c1);
    vect.push_back(c2);
    vect.push_back(c3);

    bool ans;
    ans = cellIsInVector(vect, c4);
    //FALSE answer
    std::cout << (ans ? "true" : "false") << std::endl;

    //TRUE answer
    ans = cellIsInVector(vect, c3);
    std::cout << (ans ? "true" : "false") << std::endl;

}