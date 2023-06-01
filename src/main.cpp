#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include "LKMCRun.h"

using std::chrono::time_point;
using std::chrono::system_clock;
using std::chrono::duration;

std::string getTimeString(time_point<system_clock> t) {
    std::time_t tt = std::chrono::system_clock::to_time_t(t);
    std::string stt = std::ctime(&tt);
    return stt;
}

int main(int argc, char** argv) {
    std::string input_json_file;
    if(argc == 2){
        input_json_file = static_cast<std::string> (argv[1]);
    }else{
		std::cout << "An input file path as parameter must be given, and only this one is needed and accepted." << std::endl;
		std::cout << "Example : KMC_For_Vacancy_Migration input.txt " << std::endl;
	}

    time_point<system_clock> startTime {system_clock::now()};
    std::cout << "Start Time : " << getTimeString(startTime);


    LKMCRun lkmc;
    if(lkmc.initializeLKMC(input_json_file) == true){
        lkmc.run();
    }else{
        std::cout << "LKMC Initialization Failed, Program Ending." << std::endl;
    }

    time_point<system_clock> endTime {system_clock::now()};
    duration<double> elapsedTime = endTime - startTime;

    std::cout << "End Time : " << getTimeString(endTime);
    std::cout << "Total CPU Run Time : " << elapsedTime.count() << " seconds." << std::endl;

    return 0;
}
