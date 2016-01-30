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
#include "camera.h"
#include "vector.h"
#include "material.h"
#include "surface.h"

#define __REAL_TIME__

const char * window_title = "RayTracer - OpenCL";
const char * ray_tracer_filename = "OpenCL-Mac/kernels/ray_tracer.cl";

cl_mem tex;
void set_camera_kernel_args();
vector4 get_cam_vel();
vector4 get_cam_rot();
void render_cl(float time);
void present_gl();

static struct cam_data camera;

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
	tex = clCreateFromGLTexture2D(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D,
                                       0, screen_tex, &err);
	cl_check_err(err, "clCreateFromGLTexture");

    /* -----------------------------------------------------------
     Main runtime loop. Here we will update an OpenGL texture
     buffer using OpenCL, then swap the OpenGL display buffers.
     ----------------------------------------------------------- */

    // camera data
    camera.pos      = zero_vector4();
    camera.look     = vector3_init(0.0, 0.0, 1.0);
    camera.right    = vector3_init(1.0, 0.0, 0.0);
    camera.up       = vector3_init(0.0, screen_h/(float)screen_w, 0.0);

    // surface data
    int num_surfaces = 3;
    surface * spheres = (surface *)malloc(sizeof(surface) * num_surfaces);
    spheres[0] = make_sphere(vector3_init(0, 0, 3), 0.3);
    spheres[1] = make_sphere(vector3_init(1.1, 0, 5), 1.0);
    spheres[2] = make_plane(vector3_init(0, 0, 18), vector3_init(0, 0, 1));

    material * materials = (material *)malloc(sizeof(material) * num_surfaces);
    materials[0] = rand_material();
    materials[1] = rand_material();
    materials[2] = diffuse_material(rand_vector4());

    cl_mem surfaces = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                     num_surfaces * sizeof(surface), NULL, &err);
    cl_check_err(err, "clCreateBuffer(...)");
    err = clEnqueueWriteBuffer(command_queue, surfaces, CL_TRUE, 0,
                               num_surfaces * sizeof(surface), spheres, 0, NULL, NULL);
    cl_check_err(err, "clEnqueueWriteBuffer(...)");

    cl_mem mat = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                      num_surfaces * sizeof(material), NULL, &err);
    cl_check_err(err, "clCreateBuffer(...)");
    err = clEnqueueWriteBuffer(command_queue, mat, CL_TRUE, 0,
                               num_surfaces * sizeof(material), materials, 0, NULL, NULL);
    cl_check_err(err, "clEnqueueWriteBuffer(...)");

    free(spheres);
    free(materials);

    // set up our surfaces
    err  = clSetKernelArg(kernel, 5, sizeof(cl_mem), &surfaces);
    err  = clSetKernelArg(kernel, 6, sizeof(cl_mem), &mat);
    err |= clSetKernelArg(kernel, 7, sizeof(int), &num_surfaces);

    // set the output reference
    err |= clSetKernelArg(kernel, 9, sizeof(cl_mem), &tex);
    cl_check_err(err, "clSetKernelArg(...)");

    float time = 1.8f;

#ifndef __REAL_TIME__
    render_cl(time = 3.5);
#endif

    glfwSetTime(0.0f);
    while (!glfwWindowShouldClose(window)) {
        // update the camera position from the input
        move_camera(&camera, get_cam_vel());
        rotate_camera(&camera, get_cam_rot());
        set_camera_kernel_args();

#ifdef __REAL_TIME__
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
    int seed = rand();
    vector4 light_pos = vector4_init(5 * sin(time), 0.0, 5 * cos(time) + 5.0, 0.0);
    err  = clSetKernelArg(kernel, 4, sizeof(vector4), &light_pos);
    err |= clSetKernelArg(kernel, 8, sizeof(int), &seed);
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
