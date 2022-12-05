cmake_minimum_required(VERSION 3.0.0)
project(beatstep)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories (src)

find_package(RtMidi REQUIRED)

add_subdirectory(cxxopts)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC cxxopts)
target_link_libraries(${PROJECT_NAME} PUBLIC cxxopts RtMidi::rtmidi)
