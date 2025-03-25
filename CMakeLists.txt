cmake_minimum_required(VERSION 3.10)

project(template_glfw_glad
    VERSION 1.0.0
    DESCRIPTION "Template GLFW & GLAD project"
)

add_subdirectory(C:/Users/vikto/projects/libraries/glfw-master build/glfw)
include_directories(template_glfw_glad include)
add_executable(main src/main.cpp src/base.cpp src/glad.c)
target_link_libraries(main glfw)
file(COPY resource/textures.png DESTINATION /)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")