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

const char * window_title = "RayTracer - OpenCL";
const char * ray_tracer_filename = "OpenCL-Mac/kernels/ray_tracer.cl";

/**
 Application entry point. Here we will create the OpenCL context,
 load a sample program, and test the results for a given set of data.
 */
int main(int argc, const char ** argv) {
    srand((int)time(NULL));
    int err = CL_SUCCESS;           // error code parameter for OpenCL functions

    init_gl(window_title, 1);
    init_cl(&ray_tracer_filename, 1);

	// create the OpenCL reference to our OpenGL texture
	cl_mem tex = clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D,
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
    int num_surfaces = 4;
    struct vector4 * spheres = (struct vector4 *)malloc(sizeof(struct vector4) * num_surfaces);
    for (int i = 0; i < 4; i++) {
        int x = rand() % 10 - 5;
        int y = rand() % 10 - 5;
        int z = rand() % 20 + 4;

        spheres[i] = vector4_init(x, y, z, 1.0);
    }

    cl_mem surfaces = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                     num_surfaces * sizeof(struct vector4), NULL, NULL);
    err = clEnqueueWriteBuffer(command_queue, surfaces, CL_TRUE, 0,
                               num_surfaces * sizeof(struct vector4), spheres, 0, NULL, NULL);
    cl_check_err(err, "clEnqueueWriteBuffer(...)");

    // set the camera arguments
    err = clSetKernelArg(kernel, 0, sizeof(struct vector4), &cam_pos);
    err |= clSetKernelArg(kernel, 1, sizeof(struct vector4), &cam_look);
    err |= clSetKernelArg(kernel, 2, sizeof(struct vector4), &cam_right);
    err |= clSetKernelArg(kernel, 3, sizeof(struct vector4), &cam_up);

    // set up our surfaces
    err |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &surfaces);
    err |= clSetKernelArg(kernel, 6, sizeof(int), &num_surfaces);

    // set the output references
    err |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &tex);
    cl_check_err(err, "clSetKernelArg(...)");

    const size_t global[] = {screen_w * sample_rate, screen_h * sample_rate};
    const size_t local[] = {1, 1};

    float time = 0.0f;

    while (!glfwWindowShouldClose(window)) {
		// perform the ray tracing in OpenCL
		glFinish();

        time += 2/60.0f;
        struct vector4 light_pos = vector4_init(3 * sin(time), 5.0, 3 * cos(time) + 3.0, 0.0);
        err = clSetKernelArg(kernel, 4, sizeof(struct vector4), &light_pos);
        cl_check_err(err, "clSetKernelArg(...)");

		err = clEnqueueAcquireGLObjects(command_queue, 1, &tex, 0, 0, NULL);
        cl_check_err(err, "clEnqueueAcquireGLObjects(...)");
        err = clEnqueueNDRangeKernel(command_queue, kernel, 2,
                               NULL, global, local, 0, NULL, NULL);
        cl_check_err(err, "clEnqueueNDRangeKernel(...)");
		err = clEnqueueReleaseGLObjects(command_queue, 1, &tex, 0, 0, NULL);
        cl_check_err(err, "clEnqueueReleaseGLObjects(...)");

		clFinish(command_queue);

		// refresh the OpenGL context with the new texture updates
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        update_screen();

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
