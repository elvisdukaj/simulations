cmake_minimum_required(VERSION 3.30.2 FATAL_ERROR)

add_library(pre_09_vis)
add_library(pre_09_vis::pre_09_vis ALIAS pre_09_vis)

target_link_libraries(pre_09_vis PUBLIC pre_09_vis_obj::pre_09_vis_obj)
