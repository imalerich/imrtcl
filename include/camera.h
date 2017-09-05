//
//  Created by Ian Malerich on 1/25/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

typedef struct {
    vector4 pos;
    vector4 look;
    vector4 right;
    vector4 up;
} cam_data;

/**
 Creates a new camera representation with the given parameters.
 \param field_of_view Horizontal field of view (radians) of the camera.
 \param near_dist Distance of the near plane.
 \param aspect_ratio Aspect ratio of the near plane.
 \return A camera representation at the origin looking forward (+z).
 */
cam_data init_camera(float field_of_view, float near_dist, float aspect_ratio);

/**
 Moves the camera by the input Velocity vector over
 the given time.
 \param camera The camera representation to be moved.
 \param vel The velocity vector at which to move the camera.
 */
void move_camera(cam_data * camera, vector4 vel);

/**
 Rotates the 'camera' by 'theta' radians around 'axis'.
 \param camera The camera representation to be rotated.
 \param axis The axis to be rotated around.
 \param theta Radians to rotate the camera by.
 */
void rotate_camera_axis(cam_data * camera, vector4 axis, float theta);

/**
 Rotates the 'camera' by the input quaternion.
 This method will simply rotate the cameras look, right, and up
 vectors by the given quaternion.
 \param camera The camera representation to be rotated.
 \param quat The quaternion that will be used for the rotation.
 */
void rotate_camera(cam_data * camera, vector4 quat);

/**
 Utility method to perform a rotation around the cam_up axis.
 \param camera The camera representatino to be rotated.
 \param theta Radians to rotate the camera by.
 */
void rotate_camera_yaw(cam_data * camera, float theta);

/**
 Utility method to perform a rotation around the cam_right axis.
 \param camera The camera representatino to be rotated.
 \param theta Radians to rotate the camera by.
 */
void rotate_camera_pitch(cam_data * camera, float theta);

/**
 Utility method to perform a rotation around the cam_look axis.
 \param camera The camera representatino to be rotated.
 \param theta Radians to rotate the camera by.
 */
void rotate_camera_roll(cam_data * camera, float theta);

#endif
