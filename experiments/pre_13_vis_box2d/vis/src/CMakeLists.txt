cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

add_library(pre_13_vis)
add_library(pre_13_vis::pre_13_vis ALIAS pre_13_vis)

target_link_libraries(pre_13_vis PUBLIC pre_13_vis_obj::pre_13_vis_obj)
