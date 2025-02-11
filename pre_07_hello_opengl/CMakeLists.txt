cmake_minimum_required(VERSION 3.30.2 FATAL_ERROR)

add_executable(pre_07_hello_opengl)

target_sources(pre_07_hello_opengl
        PUBLIC
        src/main.cpp

        PUBLIC FILE_SET CXX_MODULES FILES
        src/game.cpp
        src/opengl.cpp
)

target_compile_definitions(pre_07_hello_opengl PRIVATE "SDL_MAIN_USE_CALLBACKS=1" ENTT_STANDARD_CPP)
target_link_libraries(pre_07_hello_opengl PRIVATE SDL3::SDL3 EnTT::EnTT glm::glm GLEW::GLEW)
