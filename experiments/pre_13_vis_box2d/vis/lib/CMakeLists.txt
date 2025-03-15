cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

add_library(pre_13_vis_obj OBJECT)
add_library(pre_13_vis_obj::pre_13_vis_obj ALIAS pre_13_vis_obj)

target_sources(pre_13_vis_obj
        PUBLIC FILE_SET CXX_MODULES FILES
        opengl.cpp
        engine.cpp
        vis.cpp
        math.cpp
        entt.cpp
        mesh.cpp
        physic.cpp
)

target_compile_definitions(pre_13_vis_obj PUBLIC "SDL_MAIN_USE_CALLBACKS=1" ENTT_STANDARD_CPP)
target_link_libraries(pre_13_vis_obj PUBLIC SDL3::SDL3 EnTT::EnTT glm::glm GLEW::GLEW box2d::box2d)
