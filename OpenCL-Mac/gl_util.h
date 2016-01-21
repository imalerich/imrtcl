//
//  gl_util.h
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/21/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef GL_UTIL_H
#define GL_UTIL_H

#ifndef XCODE
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

extern GLFWwindow * window;
extern unsigned screen_w;
extern unsigned screen_h;

/**
 Initializes an OpenGL context using screen 
 dimmensions of screen_w and screen_h.
 \param title Title to use for the window.
 \param v_sync The swap interval, use 0 to disable vertical sync.
 */
void init_gl(const char * title, int v_sync);

/**
 Draws a rectangle over the entire screen using the
 texture to be filled by OpenCL.
 */
void update_screen();

/**
 Checks if an operation has produced an error since last
 checking for an error. If so, this method will print out
 the information detailed in 'info' as well as the error code
 to stderr, and then exit with a failure.
 */
void gl_check_errors(const char * info);

#endif
