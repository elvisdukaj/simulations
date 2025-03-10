cmake_minimum_required(VERSION 3.31.6 FATAL_ERROR)

add_library(pre_11_vis)
add_library(pre_11_vis::pre_11_vis ALIAS pre_11_vis)

target_link_libraries(pre_11_vis PUBLIC pre_11_vis_obj::pre_11_vis_obj)
