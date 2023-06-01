//
// Created by Liu Yong on 2021/8/19.
//

#include "Object.h"

Object::Object(Coord coord){
    currentCoord = coord;
    visited = false;
    eventsTable.clear();
}