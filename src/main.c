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
#include "camera.h"
#include "vector.h"
#include "material.h"
#include "surface.h"

const char * window_title = "imrtcl";
const char * ray_tracer_filename = "../kernels/ray_tracer.cl";

cl_mem tex;
void set_camera_kernel_args();
vector4 get_cam_vel();
vector4 get_cam_rot();
void render_cl(float time);
void present_gl();

static cam_data camera;

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
	// tex = clCreateFromGLTexture2D(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D,
    //                                    0, screen_tex, &err);
	tex = clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D,
		0, screen_tex, &err);
	cl_check_err(err, "clCreateFromGLTexture");

    camera = init_camera(M_PI / 2.0f, 1.0f, screen_w / (float)screen_h);

	/* --------
	 * SURFACES
	 * -------- */

    static const unsigned dimm = 3;
    int num_surfaces = dimm * dimm + 1, surf_val_count = 0;
	size_t surf_size = sizeof(cl_float) * (SPHERE_SIZE * (num_surfaces-1) + PLANE_SIZE);
    cl_float * spheres = (cl_float *)malloc(surf_size);

	cl_float * tmp = spheres;
    for (int x = 0; x < dimm; x++) {
        for (int y = 0; y < dimm; y++) {
            float x_pos = x - floor(dimm / 2);
            float y_pos = y - floor(dimm / 2);
            make_sphere(vector3_init(x_pos, y_pos, 10), 0.5, tmp);

			tmp += SPHERE_SIZE; surf_val_count += SPHERE_SIZE;
        }
    }

	make_plane(vector3_init(0, 0, 10.5), vector3_init(0, 0, 1), tmp);
	tmp += PLANE_SIZE; surf_val_count += PLANE_SIZE;

    cl_mem surfaces = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                     surf_size, NULL, &err);
    cl_check_err(err, "clCreateBuffer(...)");
    err = clEnqueueWriteBuffer(command_queue, surfaces, CL_TRUE, 0,
                               surf_size, spheres, 0, NULL, NULL);
    cl_check_err(err, "clEnqueueWriteBuffer(...)");

    free(spheres);

	/* ---------
	 * MATERIALS
	 * --------- */

	int mat_val_count = num_surfaces * sizeof(material) / sizeof(cl_float);
	material * materials = (material *)malloc(sizeof(material) * num_surfaces);
	for (int i=0; i<num_surfaces; i++) {
		materials[i] = rand_material();
	}
	materials[num_surfaces-1] = diffuse_material(vector3_init(1, 0, 0.4));

	cl_mem mat = clCreateBuffer(context, CL_MEM_READ_ONLY,
			num_surfaces * sizeof(material), NULL, &err);
	cl_check_err(err, "clCreateBuffer(...)");
	err = clEnqueueWriteBuffer(command_queue, mat, CL_TRUE, 0,
			num_surfaces * sizeof(material), materials, 0, NULL, NULL);
	cl_check_err(err, "clEnqueueWriteBuffer(...)");

	free(materials);

    // set up our surfaces
    err  = clSetKernelArg(kernel, 6, sizeof(cl_mem), &surfaces);
    err |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &mat);
	err |= clSetKernelArg(kernel, 8, surf_size, NULL);
	err |= clSetKernelArg(kernel, 9, num_surfaces * sizeof(material), NULL);
    err |= clSetKernelArg(kernel, 10, sizeof(int), &num_surfaces);
    err |= clSetKernelArg(kernel, 11, sizeof(int), &surf_val_count);
    err |= clSetKernelArg(kernel, 12, sizeof(int), &mat_val_count);

    // set the output reference
    err |= clSetKernelArg(kernel, 13, sizeof(cl_mem), &tex);
    cl_check_err(err, "clSetKernelArg(...)");

    float time = 1.8f;

#ifndef __REAL_TIME__
    glfwSetTime(0.0f);
    set_camera_kernel_args();
    render_cl(time = 3.5);
    printf("Rendered in %f seconds.\n", glfwGetTime());
#endif

    glfwSetTime(0.0f);
    while (!glfwWindowShouldClose(window)) {
#ifdef __REAL_TIME__
        // update the camera position from the input
        move_camera(&camera, get_cam_vel());
        rotate_camera(&camera, get_cam_rot());
        set_camera_kernel_args();

        render_cl(time += 2/60.0f);
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

vector4 get_cam_vel() {
    vector4 cam_vel = zero_vector4();
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        { cam_vel.x = 1.0f * time_passed; }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        { cam_vel.x = -1.0f * time_passed; }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        { cam_vel.z = 1.0f * time_passed; }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        { cam_vel.z = -1.0f * time_passed; }

    return cam_vel;
}

vector4 get_cam_rot() {
    float pitch = 0.0f, yaw = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        { pitch = 1.0f * time_passed; }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        { pitch = -1.0f * time_passed; }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        { yaw = 1.0f * time_passed; }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        { yaw = -1.0f * time_passed; }

    return zero_vector4();
}

void set_camera_kernel_args() {
    static int err = CL_SUCCESS;
    err  = clSetKernelArg(kernel, 0, sizeof(vector4), &camera.pos);
    err |= clSetKernelArg(kernel, 1, sizeof(vector4), &camera.look);
    err |= clSetKernelArg(kernel, 2, sizeof(vector4), &camera.right);
    err |= clSetKernelArg(kernel, 3, sizeof(vector4), &camera.up);
    cl_check_err(err, "clSetKernelArg(...)");
}

void render_cl(float time) {
    static int err = CL_SUCCESS;
    const size_t global[] = {screen_w * sample_rate, screen_h * sample_rate};
    const size_t local[] = {8 * sample_rate, 8 * sample_rate};

    glFinish();
    unsigned seed = rand();
#ifdef __REAL_TIME__
    vector4 light_pos = vector4_init(2 * sin(time), 2 * cos(time), 8.0, 0.0);
#else
    vector4 light_pos = vector4_init(2 * sin(time), 2 * cos(time), 8.0, 0.5);
#endif
    err |= clSetKernelArg(kernel, 4, sizeof(unsigned), &seed);
    err  = clSetKernelArg(kernel, 5, sizeof(vector4), &light_pos);
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
    glClearColor(1, 0.2, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    update_screen();
}
