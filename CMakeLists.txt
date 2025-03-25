cmake_minimum_required(VERSION 3.10)

project(voxel
    VERSION 1.0.0
    DESCRIPTION "Voxel engine"
)

add_subdirectory(../libraries/glfw-master glfw) #set glfw source directory here
include_directories(template_glfw_glad include)
add_executable(main src/main.cpp src/base.cpp src/glad.c)
target_link_libraries(main glfw)
file(COPY resource/textures.png DESTINATION /)
#link standard library if using minGW
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")