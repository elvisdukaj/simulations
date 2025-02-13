cmake_minimum_required(VERSION 3.30.2 FATAL_ERROR)

add_executable(pre_05_hello_geometry)

target_sources(pre_05_hello_geometry
        PUBLIC
        src/main.cpp

        PUBLIC FILE_SET CXX_MODULES FILES
        src/game.cpp
)

target_compile_definitions(pre_05_hello_geometry PRIVATE "SDL_MAIN_USE_CALLBACKS=1" ENTT_STANDARD_CPP)
target_link_libraries(pre_05_hello_geometry PRIVATE SDL3::SDL3 EnTT::EnTT glm::glm)
