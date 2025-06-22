cmake_minimum_required(VERSION 3.30)
project(Upscaler)

set(CMAKE_CXX_STANDARD 20)

add_executable(Upscaler main.cpp
        metrics/metrics.cpp
        metrics/metrics.h)
find_package(OpenCV REQUIRED)

target_link_libraries(Upscaler ${OpenCV_LIBS})
