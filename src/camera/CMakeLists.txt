cmake_minimum_required(VERSION 2.8.4)
project(Pharosa)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fopenmp -Wall -MMD")
add_definitions(-D CAMERA_PUSH_IN=1)

aux_source_directory(. DIR_SRCS)

message("in 'src/camera/', found sources:")
foreach(src_file ${DIR_SRCS})
    message("\t" ${src_file})
endforeach()
message("")

add_library(camera ${DIR_SRCS})
