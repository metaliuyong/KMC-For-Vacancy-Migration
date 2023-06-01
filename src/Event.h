//
// Created by Liu Yong on 2021/8/19.
//

#ifndef KMC_FOR_VACANCY_MIGRATION_EVENT_H
#define KMC_FOR_VACANCY_MIGRATION_EVENT_H

#include "Coord.h"

class Event {
public:
    Coord newCoord;
    double energyBarrier;

public:
    Event() = default;
    ~Event() = default;
    Event(Coord coords, double e);
};


#endif //KMC_FOR_VACANCY_MIGRATION_EVENT_H
