//
// Created by Liu Yong on 2021/8/19.
//

#include "Coord.h"

Coord::Coord(){
    x = 0;
    y = 0;
    index = 0;
}


Coord::Coord(int x, int y, int index){
    this->x = x;
    this->y = y;
    this->index = index;
}

bool Coord::operator==(const Coord &c){
    return (x == c.x && y == c.y && index == c.index);
}


void Coord::operator=(const Coord &c){
    x = c.x;
    y = c.y;
    index = c.index;
}