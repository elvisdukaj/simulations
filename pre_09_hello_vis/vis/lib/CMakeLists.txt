cmake_minimum_required(VERSION 3.30.2 FATAL_ERROR)

add_library(pre_09_vis_obj OBJECT)
add_library(pre_09_vis_obj::pre_09_vis_obj ALIAS pre_09_vis_obj)

target_sources(pre_09_vis_obj
        PUBLIC FILE_SET CXX_MODULES FILES
        opengl.cpp
        engine.cpp
        vis.cpp
        math.cpp
)

target_compile_definitions(pre_09_vis_obj PUBLIC "SDL_MAIN_USE_CALLBACKS=1" ENTT_STANDARD_CPP)
target_link_libraries(pre_09_vis_obj PUBLIC SDL3::SDL3 EnTT::EnTT glm::glm GLEW::GLEW)
