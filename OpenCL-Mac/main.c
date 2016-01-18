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

#ifdef XCODE
	#include <OpenCL/opencl.h>
#else
	#include <CL/cl.h>
#endif

#include "file_io.h"
#include "cl_util.h"

/// Number of elements in the data set we will work with in parallel.
const unsigned int DATA_SET_SIZE = 1024;

/**
 Application entry point. Here we will create the OpenCL context,
 load a sample program, and test the results for a given set of data.
 */
int main(int argc, const char ** argv) {
    int err = CL_SUCCESS;           // error code parameter for OpenCL functions

    float data[DATA_SET_SIZE];      // array of input data to the kernel
    float results[DATA_SET_SIZE];   // array of output data
    unsigned int correct;           // the number of correct results returned

    size_t global;                  // global domain size for the calculations
    size_t local;                   // local domain size for the calculations

    // fill the data set with random data
    for (int i = 0; i < DATA_SET_SIZE; i++) {
        data[i] = rand() / (float)RAND_MAX;
    }

    /* -----------------------------------------------------------
     Setup the OpenCL device and program representations.
        For each reference we need, we will call the appropriate
        OpenCL initialization function, then check if
        any error occurred.
     ----------------------------------------------------------- */

    // OpenCL device and program representations
    cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;
    cl_kernel kernel;

    // device memory references
    cl_mem input;
    cl_mem output;

    // connect to the compute device
    err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    check_err(err, "clGetDeviceIDs(...)");

    // create the working context
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    check_err(err, "clCreateContext(...)");

    // next up is the command queue
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    check_err(err, "clCreateCommandQueue(...)");

    // create the compute program from 'kernels.cl'
    char * buffer = read_file("/Users/imm/Development/OpenCL-Mac/OpenCL-Mac/kernels.cl");
    program = clCreateProgramWithSource(context, 1, (const char **)&buffer, NULL, &err);
    check_err(err, "clCreateProgramWithSource(...)");
    free(buffer); // program already read, we don't need the buffer anymore

    // compile the program for our device
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    check_err(err, "clBuildProgram(...)");

    // create the computer kernel in the program we wish to run
    kernel = clCreateKernel(program, "square", &err);
    check_err(err, "clCreateKernel(...)");

    // create the input and output arrays in our device memory
    input = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * DATA_SET_SIZE, NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * DATA_SET_SIZE, NULL, NULL);

    // copy our 'data' array into the compute device memory
    err = clEnqueueWriteBuffer(command_queue, input, CL_TRUE, 0, sizeof(float) * DATA_SET_SIZE,
                               data, 0, NULL, NULL);

    // set the arguments for the kernel
    // these are the actual parameters to the kernel function in kernels.cl
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &DATA_SET_SIZE);
    check_err(err, "clSetKernelArg(...)");

    // get the maximum work group size for program execution
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE,
                                   sizeof(local), &local, NULL);
    check_err(err, "clGetKernelWorkGroupInfo(...)");
    global = DATA_SET_SIZE;

    /* -----------------------------------------------------------
     Execute the kernel over the entire range of our input data,
        using the maximum number of 
        work group items for this device.
     ----------------------------------------------------------- */

    // enqueue's the commands to run on our compute device
    err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    check_err(err, "clEnqueueNDRangeKernel(...)");

    // wait for the computation to complete
    clFinish(command_queue);

    // read back the results from our compute device
    err = clEnqueueReadBuffer(command_queue, output, CL_TRUE, 0, sizeof(float) * DATA_SET_SIZE,
                              results, 0, NULL, NULL);
    check_err(err, "clEnqueueReadBuffer(...)");

    /* -----------------------------------------------------------
     Check the results of the operation and print a brief
        synopsis of the results.
     ----------------------------------------------------------- */

    correct = 0;
    for (int i = 0; i < DATA_SET_SIZE; i++) {
        correct += results[i] == data[i] * data[i];
    }

    printf("Program finished with a score of %d/%d.\n", correct, DATA_SET_SIZE);

    /* -----------------------------------------------------------
     Application cleanup.
     ----------------------------------------------------------- */

    clReleaseMemObject(input);
    clReleaseMemObject(output);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    return 0;
}
