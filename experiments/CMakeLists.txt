cmake_minimum_required(VERSION 3.31 FATAL_ERROR)
project(SIMULATION CXX)

if (BUILD_PRE_EXAMPLES)
    #    add_subdirectory(pre_01_callbacks)
    #    add_subdirectory(pre_02_rectangle)
    #    add_subdirectory(pre_03_mouse)
    #    add_subdirectory(pre_04_hello_entt)
    #    add_subdirectory(pre_05_hello_geometry)
    #    add_subdirectory(pre_06_hello_box2d)
    #    add_subdirectory(pre_07_hello_opengl)
    #    add_subdirectory(pre_08_hello_vis)
    #    add_subdirectory(pre_09_hello_vis)
    #    add_subdirectory(pre_10_vis_mesh)
    #    add_subdirectory(pre_11_vis)
    #    add_subdirectory(pre_12_vis_entt)
    add_subdirectory(pre_13_vis_box2d)
endif ()
