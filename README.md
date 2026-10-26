# 3D Cube

A C++ graphics project that renders a colored 3D cube using OpenGL and GLFW. The repository includes the source and graphics-loader files.

## Requirements

- A C++ compiler
- OpenGL development libraries
- GLFW
- GLAD
- TinyObjLoader (`tiny_obj_loader.h`)

## Build and run

There is no build-system file in the repository. Configure `cube/Source.cpp`, `cube/glad.c`, and the headers in your IDE, provide `tiny_obj_loader.h`, and link GLFW and the platform OpenGL libraries. The entry point is in `cube/Source.cpp`.
