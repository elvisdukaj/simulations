cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

add_library(pre_12_vis)
add_library(pre_12_vis::pre_12_vis ALIAS pre_12_vis)

target_link_libraries(pre_12_vis PUBLIC pre_12_vis_obj::pre_12_vis_obj)
