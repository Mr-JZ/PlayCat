cmake_minimum_required(VERSION 3.0.0)
project(PlayCat VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
find_package(Boost 1.67 REQUIRED)
include_directories(${BOOST_INCLUDE_DIRS})

set(SOURCES
    src/main.cpp
    src/CatFinder.h
    src/CatFinder.cpp
    src/MJPEGServer/MJPEGServer.h
    src/MJPEGServer/MJPEGServer.cpp
    src/LaserPointerFinder.h
    src/LaserPointerFinder.cpp
)

add_executable(PlayCat ${SOURCES})

target_link_libraries(PlayCat
    pthread
    ${OpenCV_LIBS}
    ${BOOST_LIBRARIES}
)
