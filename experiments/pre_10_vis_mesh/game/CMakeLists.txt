cmake_minimum_required(VERSION 3.30.2 FATAL_ERROR)

add_executable(pre_10_game)

target_sources(pre_10_game
        PUBLIC main.cpp
        PUBLIC FILE_SET CXX_MODULES FILES game.cpp)

target_link_libraries(pre_10_game PRIVATE pre_10_vis::pre_10_vis)