cmake_minimum_required(VERSION 3.30.2 FATAL_ERROR)

add_library(pre_10_vis)
add_library(pre_10_vis::pre_10_vis ALIAS pre_10_vis)

target_link_libraries(pre_10_vis PUBLIC pre_10_vis_obj::pre_10_vis_obj)
