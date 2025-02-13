cmake_minimum_required(VERSION 3.30.2 FATAL_ERROR)

add_executable(pre_09_game)

target_sources(pre_09_game
        PUBLIC main.cpp
        PUBLIC FILE_SET CXX_MODULES FILES game.cpp)

target_link_libraries(pre_09_game PRIVATE pre_09_vis::pre_09_vis)