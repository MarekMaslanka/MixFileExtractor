cmake_minimum_required(VERSION 3.9)
project(MixFileExtractor)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")
set(CMAKE_CXX_STANDARD 14)

include_directories(inc)

add_executable(MixFileExtractor
        inc/Palette.h
        inc/ShpLoader.h
        src/Palette.cpp
        src/MixFileExtractor.cpp
        src/ShpLoader.cpp)

target_link_libraries(MixFileExtractor png)
