cmake_minimum_required(VERSION 3.30.2 FATAL_ERROR)

add_executable(pre_04_hello_entt)

target_sources(pre_04_hello_entt
PUBLIC
  src/main.cpp

PUBLIC FILE_SET CXX_MODULES FILES
  src/game.cpp
)

target_compile_definitions(pre_04_hello_entt PRIVATE "SDL_MAIN_USE_CALLBACKS=1" ENTT_STANDARD_CPP)
target_link_libraries(pre_04_hello_entt PRIVATE SDL3::SDL3 EnTT::EnTT glm::glm)
