cmake_minimum_required(VERSION 3.30.2 FATAL_ERROR)

add_executable(pre_08_hello_vis)

target_sources(pre_08_hello_vis
        PUBLIC
        src/main.cpp

        PUBLIC FILE_SET CXX_MODULES FILES
        src/game.cpp
        src/opengl.cpp
        src/engine.cpp
        src/vis.cpp
        src/math.cpp
)

target_compile_definitions(pre_08_hello_vis PRIVATE "SDL_MAIN_USE_CALLBACKS=1" ENTT_STANDARD_CPP)
target_link_libraries(pre_08_hello_vis PRIVATE SDL3::SDL3 EnTT::EnTT glm::glm GLEW::GLEW)
