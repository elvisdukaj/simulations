cmake_minimum_required(VERSION 3.31.6 FATAL_ERROR)

add_executable(pre_11_game)

target_sources(pre_11_game
        PUBLIC main.cpp
        PUBLIC FILE_SET CXX_MODULES FILES game.cpp)

target_link_libraries(pre_11_game PRIVATE pre_11_vis::pre_11_vis)