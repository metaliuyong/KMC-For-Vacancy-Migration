//
// Created by Workstation on 2021/6/28.
//

#include "LKMCRun.h"
#include <random>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <queue>
#include <map>
#include <set>
#include "phyconst.h"

bool LKMCRun::initializeLKMC(std::string inputFile){
    dumpFile = getValueFromFile(inputFile, "dump_file");
    dumpFrequency = std::stoi(getValueFromFile(inputFile, "dump_frequency"));
    statisticFile = getValueFromFile(inputFile, "statistic_file");
    statisticFrequency = std::stoi(getValueFromFile(inputFile, "statistic_frequency"));
    temperature = std::stod(getValueFromFile(inputFile, "temperature"));
    maxTime = std::stod(getValueFromFile(inputFile, "max_time"));
    maxInterval = std::stod(getValueFromFile(inputFile, "max_interval"));
    maxUnchangedTime = std::stod(getValueFromFile(inputFile, "max_unchanged_time"));
    maxSteps = std::stoi(getValueFromFile(inputFile, "max_steps"));
    lxMax = std::stoi(getValueFromFile(inputFile, "lx_max"));
    lyMax = std::stoi(getValueFromFile(inputFile, "ly_max"));
    axis_a.x = std::stod(getValueFromFile(inputFile, "axis_a.x"));
    axis_a.y = std::stod(getValueFromFile(inputFile, "axis_a.y"));
    axis_b.x = std::stod(getValueFromFile(inputFile, "axis_b.x"));
    axis_b.y = std::stod(getValueFromFile(inputFile, "axis_b.y"));
    bias.x = std::stod(getValueFromFile(inputFile, "bias.x"));
    bias.y = std::stod(getValueFromFile(inputFile, "bias.y"));
    reactionPrefactor = std::stod(getValueFromFile(inputFile, "reaction_prefactor"));
    initialMonovacancyProportion = std::stod(getValueFromFile(inputFile, "monovacancy.init_proportion"));

    for(int i = 0; i < 3; ++i){
        for(int j = 0; j < 3; ++j){
            std::string keyString {"E["};
            keyString.append(std::to_string(i));
            keyString.append("][");
            keyString.append(std::to_string(j));
            keyString.append("]");
            energyBarriersTable[i][j] = std::stod(getValueFromFile(inputFile, keyString));
        }
    }
//    std::cout << "Dump File : " << dumpFile << std::endl;
//    std::cout << "Dump Frequency : " << dumpFrequency << std::endl;
//    std::cout << "Statistic File : " << statisticFile << std::endl;
//    std::cout << "Statistic Frequency : " << statisticFrequency << std::endl;
//    std::cout << "System Temperature : " << temperature << " K" << std::endl;
//    std::cout << "Reaction Prefactor : " << reactionPrefactor << " Hz" << std::endl;
//    std::cout << "Max Simulation Time : " << maxTime << " seconds" << std::endl;
//    std::cout << "Max Interval Time : " << maxInterval << " seconds" << std::endl;
//    std::cout << "Max Simulation Steps : " << maxSteps << std::endl;
//    std::cout << "Initial Monovacancy Proportion : " << initialMonovacancyProportion << std::endl;
//    std::cout << "Monovacancy Migration Energy Barrier : " << monovacancyMigrationEnergyBarrier << std::endl;
//    std::cout << "System Size : " << lxMax << "x" << lyMax << " primitive cells." << std::endl;
//    std::cout << "Primitive Axis : " << "axis_a(" << axis_a.x << "," << axis_a.y << "), " << "axis_b(" << axis_b.x << "," << axis_b.y << ")" << std::endl;
//    std::cout << "Bias : (" << bias.x << ", " << bias.y << ")" << std::endl;

    initializeMonovacancy(initialMonovacancyProportion);

    simulation_step = 0;
    simulation_time = 0.0;
    unchanged_time = 0.0;

    flagWriteStatistics = true;
    cutoff = 3;
    return true;
}

bool LKMCRun::run(){
    unsigned int seed {static_cast<unsigned> (time(nullptr))};
    std::default_random_engine dre {seed};
    std::uniform_real_distribution<double> di(0.0, 1.0);

    unsigned int seed_time {static_cast<unsigned> (time(nullptr))};
    std::default_random_engine dre_time {seed};
    std::uniform_real_distribution<double> di_time(0.0, 1.0);

    //Clean Old Dumped File and Old Statistic File When Start.
    std::ofstream ofs1;
    ofs1.open(dumpFile, std::ios::out);
    ofs1.close();

    std::ofstream ofs2;
    ofs2.open(statisticFile, std::ios::out);
    ofs2.close();

    bool flag_first_update_event {true};

    while(true){
        if(simulation_step % dumpFrequency == 0){
            writeDumpFrame(dumpFile, simulation_step);
        }
        if(flag_first_update_event){
            for(Object &obj : objectsTable){
                updateEventsOfObject(obj);
                flag_first_update_event = false;
            }
        }else{
            for(Object &obj : objectsTable){
                if(objectsTable[chosenObjectIndex].currentCoord.x){
                    updateEventsOfObject(obj);
                }
            }
        }
        
        computeCurrentEventsNumberAndRateSum();

//Convergence When Events List Is Empty
        if(eventsNumber == 0){
            std::cout << "LKMC End Because Of Reaching Stable Condition." << std::endl;
            break;
        }

        std::map<int, int> oldVacancyTypeCount = vacancyTypeCount;
        if(simulation_step % statisticFrequency == 0){
            writeStatisticFrame(statisticFile);
        }
        if(vacancyTypeCount != oldVacancyTypeCount){
            unchanged_time =0;
        }
        unchanged_time += residence_time;
        if(unchanged_time >= maxUnchangedTime){
            break;
        }

        ++simulation_step;

        //Convergence When Reaching Max Steps or Max Time


        if(simulation_step > maxSteps || simulation_time > maxTime ){
            break;
        }


//Convergence When Reaching Max Average Interval;
        if(1.0/rateSum >= maxInterval){
            std::cout << "LKMC End Because Of Reaching Max Average Interval between Two Neighbor Events." << std::endl;
            break;
        }

        double randomForEvent {di(dre)};

        chooseObjectIndexAndEventIndex(randomForEvent);
        realizeEvent();

        double random_for_time {di_time(dre_time)};
        residence_time = -log(random_for_time)/rateSum;
        simulation_time += residence_time;
    }

    if(simulation_time >= maxTime){
        std::cout << "LKMC End Because Of Reaching Max Simulation Time." << std::endl;
    }
    if(simulation_step >= maxSteps){
        std::cout << "LKMC End Because Of Reaching Max Simulation Step." << std::endl;
    }
    if(unchanged_time >= maxUnchangedTime){
        std::cout << "LKMC End Because No Changes of Vacancy Type Count After Max Unchanged Time." << std::endl;
    }
    return true;
}

bool LKMCRun::initializeMonovacancy(double proportion){
    unsigned int seed {static_cast<unsigned> (time(nullptr))};
    std::default_random_engine dre {seed};
    std::uniform_real_distribution<double> di(0.0, 1.0);

    for(int index = 0; index < 2; ++index){
        for(int i = 0; i < lxMax; ++i){
            for(int j = 0; j < lyMax; ++j){
                if (di(dre) < proportion) {
                    objectsTable.push_back(Object(Coord(i, j, index)));
                }
            }
        }
    }
    return true;
}

void LKMCRun::updateEventsOfObject(Object &obj){
    obj.eventsTable.clear();

    if(obj.currentCoord.index == 0){
        int num_of_nearest_neighbor_vacancies {0};

        Coord nearestCoord_1 {};
        nearestCoord_1.x = obj.currentCoord.x;
        nearestCoord_1.y = obj.currentCoord.y;
        nearestCoord_1.index = 1;

        Coord nearestCoord_2 {};
        nearestCoord_2.x = (obj.currentCoord.x + lxMax - 1) % lxMax;
        nearestCoord_2.y = obj.currentCoord.y;
        nearestCoord_2.index = 1;

        Coord nearestCoord_3 {};
        nearestCoord_3.x = (obj.currentCoord.x + lxMax - 1) % lxMax;
        nearestCoord_3.y = (obj.currentCoord.y + lyMax - 1) % lyMax;
        nearestCoord_3.index = 1;

        bool flag_1_is_vacancy = isVacancy(nearestCoord_1);
        bool flag_2_is_vacancy = isVacancy(nearestCoord_2);
        bool flag_3_is_vacancy = isVacancy(nearestCoord_3);

        std::multiset<bool> tempSet {};
        tempSet.insert(flag_1_is_vacancy);
        tempSet.insert(flag_2_is_vacancy);
        tempSet.insert(flag_3_is_vacancy);
        num_of_nearest_neighbor_vacancies = tempSet.count(true);

        if(!flag_1_is_vacancy){
            int num_of_second_neighbor_vacancies {0};
            Coord secondCoord_1 {};
            secondCoord_1.x = (obj.currentCoord.x + 1) % lxMax;
            secondCoord_1.y = obj.currentCoord.y;
            secondCoord_1.index = 0;

            Coord secondCoord_2 {};
            secondCoord_2.x = (obj.currentCoord.x + 1) % lxMax;
            secondCoord_2.y = (obj.currentCoord.y + 1) % lyMax;
            secondCoord_2.index = 0;

            if(isVacancy(secondCoord_1)){
                num_of_second_neighbor_vacancies += 1;
            }
            if(isVacancy(secondCoord_2)){
                num_of_second_neighbor_vacancies += 1;
            }
            obj.eventsTable.push_back(Event(nearestCoord_1, energyBarriersTable[num_of_nearest_neighbor_vacancies][num_of_second_neighbor_vacancies]));
        }
        if(!flag_2_is_vacancy){
            int num_of_second_neighbor_vacancies {0};
            Coord secondCoord_1 {};
            secondCoord_1.x = (obj.currentCoord.x -1 + lxMax) % lxMax;
            secondCoord_1.y = obj.currentCoord.y;
            secondCoord_1.index = 0;

            Coord secondCoord_2 {};
            secondCoord_2.x = obj.currentCoord.x;
            secondCoord_2.y = (obj.currentCoord.y + 1) % lyMax;
            secondCoord_2.index = 0;

            if(isVacancy(secondCoord_1)){
                num_of_second_neighbor_vacancies += 1;
            }
            if(isVacancy(secondCoord_2)){
                num_of_second_neighbor_vacancies += 1;
            }
            obj.eventsTable.push_back(Event(nearestCoord_2, energyBarriersTable[num_of_nearest_neighbor_vacancies][num_of_second_neighbor_vacancies]));
        }
        if(!flag_3_is_vacancy){
            int num_of_second_neighbor_vacancies {0};
            Coord secondCoord_1 {};
            secondCoord_1.x = obj.currentCoord.x;
            secondCoord_1.y = (obj.currentCoord.y - 1 + lyMax) % lyMax;
            secondCoord_1.index = 0;

            Coord secondCoord_2 {};
            secondCoord_2.x = (obj.currentCoord.x - 1 + lxMax) % lxMax;
            secondCoord_2.y = (obj.currentCoord.y - 1 + lyMax) % lyMax;
            secondCoord_2.index = 0;

            if(isVacancy(secondCoord_1)){
                num_of_second_neighbor_vacancies += 1;
            }
            if(isVacancy(secondCoord_2)){
                num_of_second_neighbor_vacancies += 1;
            }
            obj.eventsTable.push_back(Event(nearestCoord_3, energyBarriersTable[num_of_nearest_neighbor_vacancies][num_of_second_neighbor_vacancies]));
        }
    }else if(obj.currentCoord.index == 1){
        int num_of_nearest_neighbor_vacancies {0};

        Coord nearestCoord_1 {};
        nearestCoord_1.x = obj.currentCoord.x;
        nearestCoord_1.y = obj.currentCoord.y;
        nearestCoord_1.index = 0;

        Coord nearestCoord_2 {};
        nearestCoord_2.x = (obj.currentCoord.x + 1) % lxMax;
        nearestCoord_2.y = obj.currentCoord.y;
        nearestCoord_2.index = 0;

        Coord nearestCoord_3 {};
        nearestCoord_3.x = (obj.currentCoord.x + 1) % lxMax;
        nearestCoord_3.y = (obj.currentCoord.y + 1) % lyMax;
        nearestCoord_3.index = 0;

        bool flag_1_is_vacancy = isVacancy(nearestCoord_1);
        bool flag_2_is_vacancy = isVacancy(nearestCoord_2);
        bool flag_3_is_vacancy = isVacancy(nearestCoord_3);

        std::multiset<bool> tempSet {};
        tempSet.insert(flag_1_is_vacancy);
        tempSet.insert(flag_2_is_vacancy);
        tempSet.insert(flag_3_is_vacancy);
        num_of_nearest_neighbor_vacancies = tempSet.count(true);

        if(!flag_1_is_vacancy){
            int num_of_second_neighbor_vacancies {0};
            Coord secondCoord_1 {};
            secondCoord_1.x = (obj.currentCoord.x - 1 + lxMax) % lxMax;
            secondCoord_1.y = (obj.currentCoord.y - 1 + lyMax) % lyMax;
            secondCoord_1.index = 1;

            Coord secondCoord_2 {};
            secondCoord_2.x = (obj.currentCoord.x - 1 + lxMax) % lxMax;
            secondCoord_2.y = obj.currentCoord.y;
            secondCoord_2.index = 1;

            if(isVacancy(secondCoord_1)){
                num_of_second_neighbor_vacancies += 1;
            }
            if(isVacancy(secondCoord_2)){
                num_of_second_neighbor_vacancies += 1;
            }
            obj.eventsTable.push_back(Event(nearestCoord_1, energyBarriersTable[num_of_nearest_neighbor_vacancies][num_of_second_neighbor_vacancies]));
        }
        if(!flag_2_is_vacancy){
            int num_of_second_neighbor_vacancies {0};
            Coord secondCoord_1 {};
            secondCoord_1.x = (obj.currentCoord.x + 1) % lxMax;
            secondCoord_1.y = obj.currentCoord.y;
            secondCoord_1.index = 1;

            Coord secondCoord_2 {};
            secondCoord_2.x = obj.currentCoord.x;
            secondCoord_2.y = (obj.currentCoord.y - 1 + lyMax) % lyMax;
            secondCoord_2.index = 1;

            if(isVacancy(secondCoord_1)){
                num_of_second_neighbor_vacancies += 1;
            }
            if(isVacancy(secondCoord_2)){
                num_of_second_neighbor_vacancies += 1;
            }
            obj.eventsTable.push_back(Event(nearestCoord_2, energyBarriersTable[num_of_nearest_neighbor_vacancies][num_of_second_neighbor_vacancies]));
        }
        if(!flag_3_is_vacancy){
            int num_of_second_neighbor_vacancies {0};
            Coord secondCoord_1 {};
            secondCoord_1.x = obj.currentCoord.x;
            secondCoord_1.y = (obj.currentCoord.y + 1) % lyMax;
            secondCoord_1.index = 1;

            Coord secondCoord_2 {};
            secondCoord_2.x = (obj.currentCoord.x + 1) % lxMax;
            secondCoord_2.y = (obj.currentCoord.y + 1) % lyMax;
            secondCoord_2.index = 1;

            if(isVacancy(secondCoord_1)){
                num_of_second_neighbor_vacancies += 1;
            }
            if(isVacancy(secondCoord_2)){
                num_of_second_neighbor_vacancies += 1;
            }
            obj.eventsTable.push_back(Event(nearestCoord_3, energyBarriersTable[num_of_nearest_neighbor_vacancies][num_of_second_neighbor_vacancies]));
        }
    }
    return;
}


bool LKMCRun::writeDumpFrame(std::string file, int step){
    std::ofstream ofs;
    ofs.open(file, std::ios::app);
    if(!ofs.is_open()){
        std::cerr << "Can't open " << file << " !" << std::endl;
        return false;
    }
    ofs << "ITEM: TIMESTEP" << std::endl;
    ofs << step << std::endl;
    ofs << "ITEM: NUMBER OF ATOMS" << std::endl;
    ofs << lxMax*lyMax*2 << std::endl;

    ofs << "ITEM: ATOMS id type x y z" << std::endl;

    int id {0};
    Vector2d tempRealCoord {0.0, 0.0};
    for(int i = 0; i < lxMax; ++i){
        for(int j = 0; j < lyMax; ++j){
            for(int index = 0; index < 2; ++index){
                ++id;
                tempRealCoord = getRealCoord(Coord(i, j, index));
                if(isVacancy(Coord(i, j, index))){
                    ofs << id << " " << 2 << " " << tempRealCoord.x << " " << tempRealCoord.y << " " << "0.0" << std::endl;
                }else{
                    ofs << id << " " << 1 << " " << tempRealCoord.x << " " << tempRealCoord.y << " " << "0.0" << std::endl;
                }
            }
        }
    }
    ofs.close();
    return true;
}

bool LKMCRun::writeStatisticFrame(std::string file){
    std::ofstream ofs;
    ofs.open(file, std::ios::app);
    if(!ofs.is_open()){
        std::cerr << "Can't open " << file << " !" << std::endl;
        return false;
    }

    if(flagWriteStatistics == true){
        ofs << std::setiosflags(std::ios::left) << std::setw(12) << "STEP" \
            << std::setw(20) << "TIME" \
            << std::setw(20) << "dt" \
            << std::setw(12) << "EVENTS" \
            << std::setw(50) << "VACANCIES" \
            << std::endl;
        flagWriteStatistics = false;
    }

    std::cout << std::setiosflags(std::ios::left) << std::setw(12) << "STEP" \
              << std::setw(20) << "TIME" \
              << std::setw(20) << "dt" \
              << std::setw(12) << "EVENTS" \
              << std::setw(50) << "VACANCIES" \
              << std::endl;

    ofs << std::setiosflags(std::ios::left) << std::setw(12) << simulation_step \
        << std::setiosflags(std::ios::scientific) << std::setprecision(8) << std::setw(20) << simulation_time \
        << std::setw(20) << residence_time;
    ofs << std::setiosflags(std::ios::left) << std::setw(12) << eventsNumber;

    std::cout << std::setiosflags(std::ios::left) << std::setw(12) << simulation_step \
        << std::setiosflags(std::ios::scientific) << std::setprecision(8) << std::setw(20) << simulation_time \
        << std::setw(20) << residence_time;
    std::cout << std::setiosflags(std::ios::left) << std::setw(12) << eventsNumber;

    for(Object &obj : objectsTable){
        obj.visited = false;
    }
    vacancyTypeCount.clear();
    for(Object &obj : objectsTable){
        if(obj.visited == false){
            ++vacancyTypeCount[getMaxVacancyConnectionNumber(obj)];
        }
    }
    ofs << "{";
    std::cout << "{";
    for(const auto &pair : vacancyTypeCount){
        ofs << pair.first << ":" << pair.second << ", ";
        std::cout << pair.first << ":" << pair.second << ", ";
    }
    ofs << "}" << std::endl;
    std::cout << "}" << std::endl;
    ofs.close();
    return true;
}

Vector2d LKMCRun::getRealCoord(Coord coord){
    int i = coord.x;
    int j = coord.y;
    int index = coord.index;
    if(index == 0){
        return Vector2d(i * axis_a.x + j * axis_b.x, i * axis_a.y + j * axis_b.y);
    }else if(index == 1){
        return Vector2d((i + bias.x) * axis_a.x + (j + bias.y) * axis_b.x, (i + bias.x) * axis_a.y + (j + bias.y) * axis_b.y);
    }
}


void LKMCRun::realizeEvent(){
    objectsTable[chosenObjectIndex].currentCoord = objectsTable[chosenObjectIndex].eventsTable[chosenEventIndex].newCoord;
    return;
}


void  LKMCRun::computeCurrentEventsNumberAndRateSum() {
    eventsNumber = 0;
    rateSum = 0.0;
    for(const Object &obj : objectsTable){
        for(const Event &eve : obj.eventsTable){
            eventsNumber += 1;
            rateSum += getRelativeProbability(eve.energyBarrier);
        }
    }
    return;
}

void LKMCRun::chooseObjectIndexAndEventIndex(double r){
    double rateAccumulate {0.0};
    for(int i = 0; i < objectsTable.size(); ++i){
        for(int j = 0; j < objectsTable[i].eventsTable.size(); ++j){
            rateAccumulate += getRelativeProbability(objectsTable[i].eventsTable[j].energyBarrier);
            if(rateAccumulate >= rateSum * r){
                chosenObjectIndex = i;
                chosenEventIndex = j;
                return;
            }
        }
    }
    return;
}



// Auxiliary Tools

std::string LKMCRun::getValueFromFile(const std::string &file, const std::string &key){
    std::ifstream ifs;
    ifs.open(file, std::ios::in);
    if(!ifs.is_open()){
        std::cerr << "Can't open " << file << " !" << std::endl;
        return "";
    }
    std::string buff;
    std::string value;
    while(std::getline(ifs, buff)){
        auto posKey = buff.find(key, 0);
        if(posKey != std::string::npos){
            auto posEqual = buff.find("=", 0);
            value = buff.substr(posEqual + 1, buff.size() - (posEqual + 1));
        }
    }
    ifs.close();
    return value;
}

bool LKMCRun::isVacancy(Coord coord){
    for(const Object &obj : objectsTable){
        if(coord == obj.currentCoord){
            return true;
        }
    }
    return false;
}

double LKMCRun::getRelativeProbability(double energyBarrier){
    return reactionPrefactor * pow(phyconst::math_e, -energyBarrier * phyconst::elementary_charge/(phyconst::k_b*temperature));
}

bool LKMCRun::hasNeighborVacancy(Coord coord) {
    if (coord.index == 0) {
        Coord tempCoord{};

        tempCoord.x = coord.x;
        tempCoord.y = coord.y;
        tempCoord.index = 1;
        if (isVacancy(tempCoord)) {
            return true;
        }

        tempCoord.x = (coord.x + lxMax - 1) % lxMax;//Periodic Boundary Condition
        tempCoord.y = coord.y;
        tempCoord.index = 1;
        if (isVacancy(tempCoord)) {
            return true;
        }

        tempCoord.x = (coord.x + lxMax - 1) % lxMax;//Periodic Boundary Condition
        tempCoord.y = (coord.y + lyMax - 1) % lyMax;
        tempCoord.index = 1;
        if (isVacancy(tempCoord)) {
            return true;
        }
    } else if (coord.index == 1) {
        Coord tempCoord{};

        tempCoord.x = coord.x;
        tempCoord.y = coord.y;
        tempCoord.index = 0;
        if (isVacancy(tempCoord)) {
            return true;
        }

        tempCoord.x = (coord.x + 1) % lxMax;//Periodic Boundary Condition
        tempCoord.y = coord.y;
        tempCoord.index = 0;
        if (isVacancy(tempCoord)) {
            return true;
        }

        tempCoord.x = (coord.x + 1) % lxMax;//Periodic Boundary Condition
        tempCoord.y = (coord.y + 1) % lyMax;
        tempCoord.index = 0;
        if (isVacancy(tempCoord)) {
            return true;
        }
    }
    return false;
}

int LKMCRun::getMaxVacancyConnectionNumber(Object &obj){
    int count {0};
    std::queue<Object> objectsQueue;
    objectsQueue.push(obj);
    obj.visited = true;
    while(objectsQueue.empty() == false) {
        Object objTemp = objectsQueue.front();
        if (objTemp.currentCoord.index == 0) {
            for (Object &o : objectsTable) {
                Coord tempCoord{};
                tempCoord.x = objTemp.currentCoord.x;
                tempCoord.y = objTemp.currentCoord.y;
                tempCoord.index = 1;
                if (o.currentCoord == tempCoord && o.visited == false) {
                    objectsQueue.push(o);
                    o.visited = true;
                }

                tempCoord.x = (objTemp.currentCoord.x + lxMax - 1) % lxMax;//Periodic Boundary Condition
                tempCoord.y = objTemp.currentCoord.y;
                tempCoord.index = 1;
                if (o.currentCoord == tempCoord && o.visited == false) {
                    objectsQueue.push(o);
                    o.visited = true;
                }

                tempCoord.x = (objTemp.currentCoord.x + lxMax - 1) % lxMax;//Periodic Boundary Condition
                tempCoord.y = (objTemp.currentCoord.y + lyMax - 1) % lyMax;
                tempCoord.index = 1;
                if (o.currentCoord == tempCoord && o.visited == false) {
                    objectsQueue.push(o);
                    o.visited = true;
                }
            }
        }
        else if (objTemp.currentCoord.index == 1) {
            for (Object &o : objectsTable) {
                Coord tempCoord{};

                tempCoord.x = objTemp.currentCoord.x;
                tempCoord.y = objTemp.currentCoord.y;
                tempCoord.index = 0;
                if (o.currentCoord == tempCoord && o.visited == false) {
                    objectsQueue.push(o);
                    o.visited = true;
                }

                tempCoord.x = (objTemp.currentCoord.x + 1) % lxMax;//Periodic Boundary Condition
                tempCoord.y = objTemp.currentCoord.y;
                tempCoord.index = 0;
                if (o.currentCoord == tempCoord && o.visited == false) {
                    objectsQueue.push(o);
                    o.visited = true;
                }

                tempCoord.x = (objTemp.currentCoord.x + 1) % lxMax;//Periodic Boundary Condition
                tempCoord.y = (objTemp.currentCoord.y + 1) % lyMax;
                tempCoord.index = 0;
                if (o.currentCoord == tempCoord && o.visited == false) {
                    objectsQueue.push(o);
                    o.visited = true;
                }
            }
        }
        objectsQueue.pop();
        ++count;
    }
    return count;
}

bool LKMCRun::isInEachOthersCutoff(Coord coord1, Coord coord2, int cutoff){
    // return coord1.x - coord2.x
}

