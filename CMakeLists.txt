# Cmake required.
cmake_minimum_required (VERSION 2.6)
project (imrtcl)

# Version number
set (imrtcl_VERSION_MAJOR 0)
set (imrtcl_VERSION_MINOR 1)

# Set compiler and linker flags for gcc
set (GCC_COVERAGE_LINK_FLAGS "-lm -fPIC")
set (GCC_COVERAGE_COMPILE_FLAGS "-std=c++14 -Wall")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# Add the binary tree to the search path
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")

# Build the binary

add_executable(imrtcl
    src/camera.c
    src/cl_util.c
    src/file_io.c
    src/gl_util.c
    src/main.c
    src/mat4x4.c
    src/material.c
    src/surface.c
    src/vector.c
)

# Link Allegro with our library

target_link_libraries(imrtcl
    OpenCL
    glfw
    GLEW
    GL
    GLU
    X11
    Xxf86vm
    Xrandr
    Xi
    glut
)

# Add the install targets
install (TARGETS imrtcl DESTINATION bin)
