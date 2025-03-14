cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

add_executable(pre_12_game)

target_sources(pre_12_game
        PUBLIC main.cpp
        PUBLIC FILE_SET CXX_MODULES FILES game.cpp)

target_link_libraries(pre_12_game PRIVATE pre_12_vis::pre_12_vis)