# Voxel engine
A successor to the initial voxel engine (also on my profile). Instead of the single vertex buffer containing a cube, it now contains a square. Using instancing, this square is rotated depending on the face of the cube it is representing. Faces obscured by another block are not sent to the GPU. The map is generated randomly on startup using a gradient noise algorithm (similar to Perlin or Simplex noise) written by me (`gradientnoise.h`).

## Setup
Before configuring and generating makefiles, make sure that the correct directory for GLFW is set in CMakeLists.txt, i.e. replace `add_subdirectory(../libraries/glfw-master glfw)` with `add_subdirectory(path/to/glfw-master glfw)`. If you would like to statically link the C++ standard library when using MinGW, uncomment the last line in CMakeLists.txt.

## Controls
WASD controls can be used in conjunction with the mouse to navigate the map. To go jump, press space.