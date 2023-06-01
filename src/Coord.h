//
// Created by Liu Yong on 2021/8/19.
//

#ifndef KMC_FOR_VACANCY_MIGRATION_COORD_H
#define KMC_FOR_VACANCY_MIGRATION_COORD_H


class Coord {
public:
    int x;
    int y;
    int index;
public:
    Coord();
    Coord(int x, int y, int index);
    ~Coord() = default;

    bool operator==(const Coord &c);
    void operator=(const Coord &c);
};


#endif //KMC_FOR_VACANCY_MIGRATION_COORD_H
