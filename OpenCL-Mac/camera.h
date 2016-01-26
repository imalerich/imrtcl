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
    struct vector4 pos;
    struct vector4 look;
    struct vector4 right;
    struct vector4 up;
};

/**
 Moves the camera by the input Velocity vector over
 the given time.
 \param camera The camera representation to be moved.
 \param vel The velocity vector at which to move the camera.
 */
void move_camera(struct cam_data * camera, struct vector4 vel);

/**
 Rotates the 'camera' by 'theta' radians around 'axis'.
 \param camera The camera representation to be rotated.
 \param axis The axis to be rotated around.
 \param theta Radians to rotate the camera by.
 */
void rotate_camera_axis(struct cam_data * camera, struct vector4 axis, float theta);

/**
 Rotates the 'camera' by the input quaternion.
 This method will simply rotate the cameras look, right, and up
 vectors by the given quaternion.
 \param camera The camera representation to be rotated.
 \param quat The quaternion that will be used for the rotation.
 */
void rotate_camera(struct cam_data * camera, struct vector4 quat);

/**
 Utility method to perform a rotation around the cam_up axis.
 \param camera The camera representatino to be rotated.
 \param theta Radians to rotate the camera by.
 */
void rotate_camera_yaw(struct cam_data * camera, float theta);

/**
 Utility method to perform a rotation around the cam_right axis.
 \param camera The camera representatino to be rotated.
 \param theta Radians to rotate the camera by.
 */
void rotate_camera_pitch(struct cam_data * camera, float theta);

/**
 Utility method to perform a rotation around the cam_look axis.
 \param camera The camera representatino to be rotated.
 \param theta Radians to rotate the camera by.
 */
void rotate_camera_roll(struct cam_data * camera, float theta);

#endif
