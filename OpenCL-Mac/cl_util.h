//
//  cl_util.h
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/17/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef CL_UTIL_H
#define CL_UTIL_H

#include <stdio.h>

#ifdef XCODE
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
	#include <CL/cl_gl.h>
#endif

// OpenCL device and program representations
extern cl_device_id device_id;
extern cl_context context;
extern cl_command_queue command_queue;
extern cl_program program;
extern cl_kernel kernel;

/**
 Utility method for checking an error code as returned by 
 an OpenCL function. If that error code is not successful,
 then check_err(...) will print the msg and terminate
 with EXIT_FAILURE, if err is CL_SUCCESS, no such
 action will be performed.
 
 \param err Error code to test.
 \param msg Message to output if an error occurs.
 \return void
 */
void check_err(int err, const char * msg);

/**
 Setup the OpenCL device and program representations.
For each reference we need, we will call the appropriate
 OpenCL initialization function, then check if
 any error occurred.
 
 \param sources Array of file names to be used for the program.
 \param count The number of items in the input array.
 */
void init_cl(const char ** sources, int count);

/**
 Releases all memory that was allocated by the
 init_cl function call.
 */
void release_cl();

#endif
