cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(SIMULATION CXX)

option(BUILD_PRE_EXAMPLES "Build the pre examples" ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)


if (BUILD_PRE_EXAMPLES)
    add_subdirectory(pre_01_callbacks)
    add_subdirectory(pre_02_rectangle)
    add_subdirectory(pre_03_mouse)
    add_subdirectory(pre_04_hello_entt)
    add_subdirectory(pre_05_hello_geometry)
    add_subdirectory(pre_06_hello_box2d)
    add_subdirectory(pre_07_hello_opengl)
endif ()
