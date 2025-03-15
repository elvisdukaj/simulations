cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

add_executable(pre_13_game)

target_sources(pre_13_game
        PUBLIC main.cpp
        PUBLIC FILE_SET CXX_MODULES FILES game.cpp)

target_link_libraries(pre_13_game PRIVATE pre_13_vis::pre_13_vis)