//
//  camera.c
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/25/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include "camera.h"

void move_camera(struct cam_data * camera, vector4 vel) {
    camera->pos.x += vel.x;
    camera->pos.y += vel.y;
    camera->pos.z += vel.z;
    camera->pos.w += vel.w;
}

void rotate_camera_axis(struct cam_data * camera, vector4 axis, float theta) {
    // TODO
}

void rotate_camera(struct cam_data * camera, vector4 quat) {
    // TODO
}

void rotate_camera_yaw(struct cam_data * camera, float theta) {
    rotate_camera_axis(camera, camera->up, theta);
}

void rotate_camera_pitch(struct cam_data * camera, float theta) {
    rotate_camera_axis(camera, camera->right, theta);
}

void rotate_camera_roll(struct cam_data * camera, float theta) {
    rotate_camera_axis(camera, camera->look, theta);
}