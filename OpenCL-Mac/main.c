//
//  main.c
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/16/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "gl_util.h"
#include "cl_util.h"
#include "vector.h"

#define __REAL_TIME__

const char * window_title = "RayTracer - OpenCL";
const char * ray_tracer_filename = "OpenCL-Mac/kernels/ray_tracer.cl";

cl_mem tex;
void render_cl(float time);
void present_gl();

/**
 Application entry point. Here we will create the OpenCL context,
 load a sample program, and test the results for a given set of data.
 */
int main(int argc, const char ** argv) {
    srand((int)time(NULL));
    int err = CL_SUCCESS;           // error code parameter for OpenCL functions

    init_gl(window_title, 0);
    init_cl(&ray_tracer_filename, 1);

	// create the OpenCL reference to our OpenGL texture
	tex = clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D,
                                       0, screen_tex, &err);
	cl_check_err(err, "clCreateFromGLTexture");

    /* -----------------------------------------------------------
     Main runtime loop. Here we will update an OpenGL texture
     buffer using OpenCL, then swap the OpenGL display buffers.
     ----------------------------------------------------------- */

    // camera data
    struct vector4 cam_pos = vector4();
    struct vector4 cam_look = vector3_init(0.0, 0.0, 1.0);
    struct vector4 cam_right = vector3_init(1.0, 0.0, 0.0);
    struct vector4 cam_up = vector3_init(0.0, screen_h/(float)screen_w, 0.0);

    // surface data
    int num_surfaces = 2;
    struct vector4 * spheres = (struct vector4 *)malloc(sizeof(struct vector4) * num_surfaces);
    spheres[0] = vector4_init(-0, 0, 3, 0.3);
    spheres[1] = vector4_init( 1.1, 0, 5, 1.0);

    cl_mem surfaces = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                     num_surfaces * sizeof(struct vector4), NULL, &err);
    cl_check_err(err, "clCreateBuffer(...)");
    err = clEnqueueWriteBuffer(command_queue, surfaces, CL_TRUE, 0,
                               num_surfaces * sizeof(struct vector4), spheres, 0, NULL, NULL);
    cl_check_err(err, "clEnqueueWriteBuffer(...)");

    free(spheres);

    // set the camera arguments
    err  = clSetKernelArg(kernel, 0, sizeof(struct vector4), &cam_pos);
    err |= clSetKernelArg(kernel, 1, sizeof(struct vector4), &cam_look);
    err |= clSetKernelArg(kernel, 2, sizeof(struct vector4), &cam_right);
    err |= clSetKernelArg(kernel, 3, sizeof(struct vector4), &cam_up);

    // set up our surfaces
    err |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &surfaces);
    err |= clSetKernelArg(kernel, 6, sizeof(int), &num_surfaces);

    // set the output reference
    err |= clSetKernelArg(kernel, 8, sizeof(cl_mem), &tex);
    cl_check_err(err, "clSetKernelArg(...)");

    float time = 1.8f;

#ifndef __REAL_TIME__
    render_cl(time = 3.5);
#endif

    while (!glfwWindowShouldClose(window)) {
#ifdef __REAL_TIME__
        render_cl(time += 1/60.0f);
#endif
        present_gl();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    /* -----------------------------------------------------------
     Application cleanup.
     ----------------------------------------------------------- */

    clReleaseMemObject(surfaces);
    release_cl();

    return 0;
}

void render_cl(float time) {
    static int err = CL_SUCCESS;
    const size_t global[] = {screen_w * sample_rate, screen_h * sample_rate};
    const size_t local[] = {8 * sample_rate, 8 * sample_rate};

    glFinish();
    int seed = rand();
    struct vector4 light_pos = vector4_init(10 * sin(time), 0.0, 10 * cos(time) + 5.0, 1.0);
    err  = clSetKernelArg(kernel, 4, sizeof(struct vector4), &light_pos);
    err |= clSetKernelArg(kernel, 7, sizeof(int), &seed);
    cl_check_err(err, "clSetKernelArg(...)");

    err = clEnqueueAcquireGLObjects(command_queue, 1, &tex, 0, 0, NULL);
    cl_check_err(err, "clEnqueueAcquireGLObjects(...)");
    err = clEnqueueNDRangeKernel(command_queue, kernel, 2,
                                 NULL, global, local, 0, NULL, NULL);
    cl_check_err(err, "clEnqueueNDRangeKernel(...)");
    err = clEnqueueReleaseGLObjects(command_queue, 1, &tex, 0, 0, NULL);
    cl_check_err(err, "clEnqueueReleaseGLObjects(...)");

    clFinish(command_queue);
}

void present_gl() {
    // refresh the OpenGL context with the new texture updates
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    update_screen();
}
