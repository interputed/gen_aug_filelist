cmake_minimum_required(VERSION 2.8)
project(gen_aug_filelist CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
set(CXX_OPTIMIZATION_FLAGS "-O2 -ffast-math -funroll-loops -ftree-vectorize")
set(CXX_ERROR_FLAGS "-Wall -Werror -Wextra -pedantic -g")
set(CMAKE_CXX_FLAGS "${CXX_OPTIMIZATION_FLAGS} ${CXX_ERROR_FLAGS}")
set(SOURCE_FILES main.cpp)

add_executable(gen_aug_filelist ${SOURCE_FILES})

target_link_libraries(gen_aug_filelist
        boost_program_options
        boost_filesystem
        boost_system
        )