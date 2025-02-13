cmake_minimum_required(VERSION 3.30.2 FATAL_ERROR)

add_executable(pre_06_hello_box2d)

target_sources(pre_06_hello_box2d
        PUBLIC
        src/main.cpp

        PUBLIC FILE_SET CXX_MODULES FILES
        src/game.cpp
)

target_compile_definitions(pre_06_hello_box2d PRIVATE "SDL_MAIN_USE_CALLBACKS=1" ENTT_STANDARD_CPP)
target_link_libraries(pre_06_hello_box2d PRIVATE SDL3::SDL3 EnTT::EnTT glm::glm box2d::box2d)
