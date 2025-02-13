cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

add_executable(pre_03_mouse)

target_sources(pre_03_mouse
        PUBLIC
        src/main.cpp

        PUBLIC FILE_SET CXX_MODULES FILES
        src/game.cpp
)

target_compile_definitions(pre_03_mouse PRIVATE "SDL_MAIN_USE_CALLBACKS=1")

target_link_libraries(pre_03_mouse PRIVATE SDL3::SDL3)
