cmake_minimum_required(VERSION 3.8)
project(opencv_test)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)

find_package(OpenCV REQUIRED)

include_directories(
        .
        ${OpenCV_INCLUDE_DIRECTORIES}
)

add_executable(opencv_test ${SOURCE_FILES})

target_link_libraries(opencv_test ${OpenCV_LIBRARIES})