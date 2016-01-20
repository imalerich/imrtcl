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

#include "cl_util.h"
#include "vector.h"

// the number of particles in our simulation
const unsigned NUM_PARTICLES    = 256;
const unsigned N_STEP           = 100000;
const unsigned N_BURST          = N_STEP / 100;

const float delta_time  = 0.01;
const float eps         = 0.01;

/**
 Given an input array of positions and velocities of size 'n',
 this function will initialize those arrays with random 
 initial values.
 */
void nbody_init(const unsigned n, struct vector4 * pos, struct vector4 * vel) {
    // the ranges allowed for the random initialization
    const static float min_x = -1000.0f;
    const static unsigned width_x = 2000.0f;;
    const static float min_y = -1000.0f;
    const static unsigned width_y = 2000.0f;
    const static unsigned max_mass = 1000;

    // initialize each particle with a random position, mass, and velocity
    for (unsigned i = 0; i < n; i++) {
        pos[i] = vector4_init(rand() % width_x + min_x, // x-pos
                              rand() % width_y + min_y, // y-pos
                              0.0f,                     // z-pos
                              rand() % max_mass);       // mass
        vel[i] = vector4_init(rand() % 10,
                              rand() % 10,
                              0.0f,
                              0.0f);  // initial velocity
    }
}

/**
 This function will write each position to the input file.
 The format will be a csv file, with one particle per line,
 with each column containing two columns, the first for the 
 x coordinate of the particle, and the second containing
 the y coordinate of the particle.
 */
void nbody_output(const char * file_name, const unsigned n, struct vector4 * pos) {
    FILE * f = fopen(file_name, "w");

    // write all simulated positions to a csv file
    for (unsigned i = 0; i < n; i++) {
        fprintf(f, "%f, %f\n", pos[i].x, pos[i].y);
    }

    fclose(f);
}

/**
 Allocates memory on the compute device that will be
 used to run the simulation.
 */
void init_memory(cl_mem * pos_old, cl_mem * pos_new, cl_mem * cur_vel) {
    // initialize the data arrays
    int err = CL_SUCCESS;
    struct vector4 pos1[NUM_PARTICLES];
    struct vector4 vel[NUM_PARTICLES];

    nbody_init(NUM_PARTICLES, pos1, vel);

    // copy our 'data' array into the compute device memory
    size_t size = sizeof(struct vector4) * NUM_PARTICLES;
    err = clEnqueueWriteBuffer(command_queue, *pos_old, CL_TRUE, 0, size, pos1, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(command_queue, *pos_new, CL_TRUE, 0, size, pos1, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(command_queue, *cur_vel, CL_TRUE, 0, size, vel, 0, NULL, NULL);
    check_err(err, "clEnqueueWriteBuffer(...)");
}

void output_memory(const char * file_name, cl_mem pos_old) {
    int err = CL_SUCCESS;
    struct vector4 pos1[NUM_PARTICLES];

    // read back the results from our compute device
    size_t size = sizeof(struct vector4) * NUM_PARTICLES;
    err = clEnqueueReadBuffer(command_queue, pos_old, CL_TRUE, 0, size, pos1, 0, NULL, NULL);
    check_err(err, "clEnqueueReadBuffer(...)");

    clFinish(command_queue);
    nbody_output(file_name, NUM_PARTICLES, pos1);
}

/**
 Application entry point. Here we will create the OpenCL context,
 load a sample program, and test the results for a given set of data.
 */
int main(int argc, const char ** argv) {
    int err = CL_SUCCESS;           // error code parameter for OpenCL functions
    size_t global;                  // global domain size for the calculations
    size_t local;                   // local domain size for the calculations

    const char * file_name = "ray_tracer.cl";
    init_cl(&file_name, 1);

    // device memory references
    cl_mem pos_old;
    cl_mem pos_new;
    cl_mem cur_vel;

    size_t size = sizeof(struct vector4) * NUM_PARTICLES;
    pos_old = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, NULL);
    pos_new = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, NULL);
    cur_vel = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, NULL);

    init_memory(&pos_old, &pos_new, &cur_vel);

    // set the arguments for the kernel
    // these are the actual parameters to the kernel function in kernels.cl
    err = clSetKernelArg(kernel, 0, sizeof(float), &delta_time);
    err |= clSetKernelArg(kernel, 1, sizeof(float), &eps);
    err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &cur_vel);
    err |= clSetKernelArg(kernel, 5, size, NULL);
    check_err(err, "clSetKernelArg(...)");

    // get the maximum work group size for program execution
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE,
                                   sizeof(local), &local, NULL);
    check_err(err, "clGetKernelWorkGroupInfo(...)");
    global = NUM_PARTICLES;

    /* -----------------------------------------------------------
     Execute the kernel over the entire range of our input data,
        using the maximum number of 
        work group items for this device.
     ----------------------------------------------------------- */

    // enqueue's the commands to run on our compute device
    check_err(err, "clEnqueueNDRangeKernel(...)");

    clFinish(command_queue);
    for (unsigned step = 0; step < N_STEP; step += N_BURST) {
        for (unsigned burst = 0; burst < N_BURST; burst += 2) {
            // tick
            err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &pos_old);
            err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &pos_new);
            check_err(err, "clSetKernelArg(...)");

            err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
                                         &global, &local, 0, NULL, NULL);
            check_err(err, "clEnqueueNDRangeKernel(...)");
            clFinish(command_queue);

            // tock
            err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &pos_new);
            err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &pos_old);
            check_err(err, "clSetKernelArg(...)");

            err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
                                         &global, &local, 0, NULL, NULL);
            check_err(err, "clEnqueueNDRangeKernel(...)");
            clFinish(command_queue);
        }
    }

    /* -----------------------------------------------------------
     Check the results of the operation and print a brief
        synopsis of the results.
     ----------------------------------------------------------- */

    clFinish(command_queue);
    output_memory("results.csv", pos_old);
    printf("success!\n");

    /* -----------------------------------------------------------
     Application cleanup.
     ----------------------------------------------------------- */

    clReleaseMemObject(pos_old);
    clReleaseMemObject(pos_new);
    clReleaseMemObject(cur_vel);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    return 0;
}
