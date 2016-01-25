//
//  camera.h
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/25/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

struct cam_data {
    struct vector4 cam_pos;
    struct vector4 cam_look;
    struct vector4 cam_right;
    struct vector4 cam_up;
};


/**
 Moves the camera by the input Velocity vector over
 the given time.
 \param camera The camera representation to be moved.
 \param vel The velocity vector at which to move the camera.
 \param delta Time interval in which to move the camera.
 */
void move_camera(struct cam_data * camera, struct vector4 vel, float delta);

#endif
