//
//  cl_util.h
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/17/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef CL_UTIL_H
#define CL_UTIL_H

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

#endif
