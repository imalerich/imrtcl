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

/**
 Application entry point. Here we will create the OpenCL context,
 load a sample program, and test the results for a given set of data.
 */
int main(int argc, const char ** argv) {
    int err = CL_SUCCESS;           // error code parameter for OpenCL functions
    size_t global;                  // global domain size for the calculations
    size_t local;                   // local domain size for the calculations

    init_gl("RayTracer - OpenCL", 1);

    const char * file_name = "OpenCL-Mac/kernels/ray_tracer.cl";
    init_cl(&file_name, 1);

    // get the maximum work group size for program execution
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE,
                                   sizeof(local), &local, NULL);
    check_err(err, "clGetKernelWorkGroupInfo(...)");
    global = 0; // TODO

    /* -----------------------------------------------------------
     Main runtime loop. Here we will update an OpenGL texture
     buffer using OpenCL, then swap the OpenGL display buffers.
     ----------------------------------------------------------- */

    while (!glfwWindowShouldClose(window)) {
        // TODO
        glClearColor(1, 0.5, 1, 1);
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
