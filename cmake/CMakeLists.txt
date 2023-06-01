cmake_minimum_required(VERSION 3.14)
project(KMC_For_Vacancy_Migration)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_INSTALL_PREFIX $ENV{HOME}/.local)

get_filename_component(KMC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)

set(KMC_SOURCE_DIR ${KMC_DIR}/src)
file(GLOB ALL_SOURCES ${KMC_SOURCE_DIR}/[^.]*.cpp)
file(GLOB MAIN_SOURCES ${KMC_SOURCE_DIR}/main.cpp)
list(REMOVE_ITEM ${ALL_SOURCES} ${MAIN_SOURCES})

add_library(KMC_SHARED_LIBRARY SHARED ${ALL_SOURCES})
add_executable(KMC_For_Vacancy_Migration ${MAIN_SOURCES})
target_link_libraries(KMC_For_Vacancy_Migration KMC_SHARED_LIBRARY)

install(TARGETS KMC_SHARED_LIBRARY KMC_For_Vacancy_Migration 
		LIBRARY DESTINATION lib
		RUNTIME DESTINATION bin
)
