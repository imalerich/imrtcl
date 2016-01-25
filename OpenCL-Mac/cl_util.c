//
//  cl_util.c
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/17/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include <string.h>

#include "gl_util.h"
#include "cl_util.h"
#include "file_io.h"

cl_device_id device_id = 0;
cl_context context;
cl_command_queue command_queue;
cl_program program;
cl_kernel kernel;

void cl_check_err(int err, const char * msg) {
    // err is not succesfull => print the error and exit
    if (err != CL_SUCCESS) {
        fprintf(stderr, "err: %d\n", err);
        fprintf(stderr, "\t%s\n", msg);
        exit(EXIT_FAILURE);
    }
}

void init_cl(const char ** sources, int count) {
    int err = CL_SUCCESS;

    // get the platform id for this system
    cl_platform_id platform;
    err = clGetPlatformIDs(1, &platform, NULL);
    cl_check_err(err, "clGetPlatformIDs(...)");

    // connect to the compute device
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    cl_check_err(err, "clGetDeviceIDs(...)");

    // set the platform specific context properties for OpenGL + OpenCL sharing
#ifdef __APPLE__
    cl_context_properties ctx_prop[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)CGLGetShareGroup(CGLGetCurrentContext()),
        0};

#endif
#ifdef __linux__
    cl_context_properties ctx_prop[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetGLXContext(window),
        CL_GLX_DISPLAY_KHR, (cl_context_properties)glfwGetX11Display(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
        0};
#endif
#ifdef __MINGW32__
    cl_context_properties ctx_prop[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
        0};
#endif

    // create the working context
    context = clCreateContext(ctx_prop, 1, &device_id, NULL, NULL, &err);
    cl_check_err(err, "clCreateContext(...)");

    // next up is the command queue
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    cl_check_err(err, "clCreateCommandQueue(...)");

    // create a single source buffer for the program from the input files
    size_t length = 0;
    for (int i = 0; i < count; i++) {
        length += file_length(sources[i]);
    }

    char * buffer = (char *)malloc(length);
    buffer[0] = '\0';
    for (int i = 0; i < count; i++) {
        char * tmp = read_file(sources[i]);
        strcat(buffer, tmp);
        free(tmp);
    }

    // create the compute program from 'kernels.cl'
    program = clCreateProgramWithSource(context, 1, (const char **)&buffer, NULL, &err);
    cl_check_err(err, "clCreateProgramWithSource(...)");
    free(buffer); // program already read, we don't need the buffer anymore

    // compile the program for our device
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    cl_check_err(err, "clBuildProgram(...)");

    // create the computer kernel in the program we wish to run
    kernel = clCreateKernel(program, "ray_tracer", &err);
    cl_check_err(err, "clCreateKernel(...)");
}

void release_cl() {
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
}
