//
// Created by Workstation on 2021/6/28.
//

#ifndef KMC_FOR_VACANCY_MIGRATION_LKMCRUN_H
#define KMC_FOR_VACANCY_MIGRATION_LKMCRUN_H

#include "Object.h"
#include "Event.h"
#include "Vector2d.h"
#include <map>
#include <array>
#include <tuple>

class LKMCRun {
public:
    std::string dumpFile;
    int dumpFrequency;
    std::string statisticFile;
    int statisticFrequency;
    double temperature;
    double reactionPrefactor;
    double maxTime;
    double maxInterval;
    double maxUnchangedTime;
    int maxSteps;
    int lxMax;
    int lyMax;
    int cutoff;
    int eventsNumber;
    double rateSum;
    int chosenObjectIndex;
    int chosenEventIndex;
    double initialMonovacancyProportion;

    int simulation_step;
    double unchanged_time;
    double simulation_time;
    double residence_time;

    bool flagWriteStatistics;

    std::array<std::array<double, 3> ,3> energyBarriersTable;

    Vector2d axis_a;
    Vector2d axis_b;
    Vector2d bias;

    std::vector<Object> objectsTable;
    std::map<int, int> vacancyTypeCount;

public:
    LKMCRun()=default;
    ~LKMCRun()=default;

    bool initializeLKMC(std::string inputFile);

    bool initializeMonovacancy(double proportion);

    void updateEventsOfObject(Object &obj);


    bool run();

    bool writeDumpFrame(std::string file, int step);

    bool writeStatisticFrame(std::string file);

    void realizeEvent();
    void chooseObjectIndexAndEventIndex(double r);

// Auxiliary Tools
    std::string getValueFromFile(const std::string &file, const std::string &key);

    Vector2d getRealCoord(Coord coord);

    bool isVacancy(Coord coord);

    void computeCurrentEventsNumberAndRateSum();

    double getRelativeProbability(double energyBarrier);

    int getMaxVacancyConnectionNumber(Object &obj);

    bool hasNeighborVacancy(Coord coord);

    bool isInEachOthersCutoff(Coord coord1, Coord coord2);
};


#endif //KMC_FOR_VACANCY_MIGRATION_LKMCRUN_H
