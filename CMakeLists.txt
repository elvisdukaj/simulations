cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(SIMULATION CXX)

option(BUILD_PRE_EXAMPLES "Build the pre examples" ON)

if (BUILD_PRE_EXAMPLES)
  add_subdirectory(pre_01_callbacks)
  add_subdirectory(pre_02_rectangle)
  add_subdirectory(pre_03_mouse)
endif()
