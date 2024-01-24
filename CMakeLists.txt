cmake_minimum_required(VERSION 2.6)
project(ArmorDetect)
set(CMAKE_CXX_FLAGS "-std=c++11")

find_package(OpenCV REQUIRED) 
include_directories(${OpenCV_INCLUDE_DIRS}) 

add_executable(ArmorDetect src/ArmorDetect.cpp)
add_executable(balltrack src/balltrack.cpp)
target_link_libraries(ArmorDetect ${OpenCV_LIBS})
target_link_libraries(balltrack ${OpenCV_LIBS})

