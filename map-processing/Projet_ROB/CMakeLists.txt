cmake_minimum_required(VERSION 3.9)
project(Projet_ROB)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)

add_executable(Projet_ROB
        src/Projet_ROB.cpp src/functions.cpp src/functions.h src/Projet_ROB.h)

include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(Projet_ROB ${OpenCV_LIBS})