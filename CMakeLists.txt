cmake_minimum_required(VERSION 3.8)
project(SabotageDetect)

set(TARGET_NAME "${PROJECT_NAME}")
set(CMAKE_CXX_STANDARD 20)
set(INCLUDE_CV /usr/include/opencv4)

find_package(OpenCV REQUIRED)

add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/blur.cpp)

include_directories( ${INCLUDE_CV} )
target_link_libraries(${TARGET_NAME} ${OpenCV_LIBS})