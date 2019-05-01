cmake_minimum_required(VERSION 3.5)
project(opencv_test)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include common.h")

find_package(OpenCV REQUIRED)

include_directories(
        .
        ${OpenCV_INCLUDE_DIRECTORIES}
)

link_libraries("glog")

# my modules
add_library(hardware "hardware/motor.cpp" "hardware/underpan.cpp")
target_link_libraries(hardware "wiringPi" "pthread")
add_library(cv "cv/findCircle.cpp")
target_link_libraries(cv ${OpenCV_LIBRARIES})
add_library(control "control/pid.cpp")

add_executable(opencv_test ${SOURCE_FILES})
target_link_libraries(opencv_test "hardware" "cv" "control")