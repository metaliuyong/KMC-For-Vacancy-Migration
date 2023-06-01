//
// Created by Liu Yong on 2021/8/19.
//

#ifndef KMC_FOR_VACANCY_MIGRATION_OBJECT_H
#define KMC_FOR_VACANCY_MIGRATION_OBJECT_H

#include <string>
#include <vector>
#include "Coord.h"
#include "Event.h"

class Object {
public:
    Coord currentCoord;
    std::vector<Event> eventsTable;
    bool visited;
public:
    Object()=default;
    ~Object()=default;
    Object(Coord coord);
};


#endif //KMC_FOR_VACANCY_MIGRATION_OBJECT_H
