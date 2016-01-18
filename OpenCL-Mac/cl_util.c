//
//  cl_util.c
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/17/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include <stdio.h>

#ifdef XCODE
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif

#include "cl_util.h"

void check_err(int err, const char * msg) {
    // err is not succesfull => print the error and exit
    if (err != CL_SUCCESS) {
        printf("%s\n", msg);
        exit(EXIT_FAILURE);
    }
}