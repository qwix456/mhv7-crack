cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(hackproldr VERSION 0.1.0 LANGUAGES C CXX)

add_library(hackproldr SHARED main.cpp)

target_include_directories(hackproldr PRIVATE
    minhook/include
)

add_subdirectory(minhook)
target_link_libraries(hackproldr minhook)