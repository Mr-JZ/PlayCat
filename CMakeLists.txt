cmake_minimum_required(VERSION 3.0.0)
project(PlayCat VERSION 0.1.0)

#include(CTest)
#enable_testing()

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCES
    src/main.cpp
    src/CatFinder.h
    src/CatFinder.cpp
    src/MJPEGWriter/MJPEGWriter.h
    src/MJPEGWriter/MJPEGWriter.cpp
    src/LaserPointerFinder.h
    src/LaserPointerFinder.cpp
)

add_executable(PlayCat ${SOURCES})

target_link_libraries(PlayCat
    ${OpenCV_LIBS}
    pthread
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
