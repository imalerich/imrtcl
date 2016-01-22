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
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "gl_util.h"
#include "cl_util.h"
#include "vector.h"

const char * window_title = "RayTracer - OpenCL";
const char * ray_tracer_filename = "OpenCL-Mac/kernels/gl_sample.cl";

/**
 Application entry point. Here we will create the OpenCL context,
 load a sample program, and test the results for a given set of data.
 */
int main(int argc, const char ** argv) {
    int err = CL_SUCCESS;           // error code parameter for OpenCL functions
//    size_t local;                   // local domain size for the calculations

    init_gl(window_title, 1);
    init_cl(&ray_tracer_filename, 1);

    // get the maximum work group size for program execution
//    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE,
//                                   sizeof(local), &local, NULL);
    cl_check_err(err, "clGetKernelWorkGroupInfo(...)\n");

	// create the OpenCL reference to our OpenGL texture
	cl_mem tex = clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D,
                                       0, screen_tex, &err);
	cl_check_err(err, "clCreateFromGLTexture");

    /* -----------------------------------------------------------
     Main runtime loop. Here we will update an OpenGL texture
     buffer using OpenCL, then swap the OpenGL display buffers.
     ----------------------------------------------------------- */

    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &tex);
    cl_check_err(err, "clSetKernelArg(...)");

    const size_t global[] = {screen_w * sample_rate, screen_h * sample_rate};
    const size_t local[] = {1, 1};
    float delta = 0.0;

    while (!glfwWindowShouldClose(window)) {
		// perform the ray tracing in OpenCL
		glFinish();

        delta += 1/120.0f;
        delta = delta > 1.0 ? 1.0 : delta;

        err = clSetKernelArg(kernel, 0, sizeof(float), &delta);
        cl_check_err(err, "clSetKernelArg(...)");

		err = clEnqueueAcquireGLObjects(command_queue, 1, &tex, 0, 0, NULL);
        cl_check_err(err, "clEnqueueAcquireGLObjects(...)");

        err = clEnqueueNDRangeKernel(command_queue, kernel, 2,
                               NULL, global, local, 0, NULL, NULL);
        cl_check_err(err, "clEnqueueNDRangeKernel(...)");

		clEnqueueReleaseGLObjects(command_queue, 1, &tex, 0, 0, NULL);
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

    release_cl();

    return 0;
}
