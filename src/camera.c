//
//  Created by Ian Malerich on 1/25/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include <math.h>
#include "camera.h"

cam_data init_camera(float field_of_view, float near_dist, float aspect_ratio) {
    float half_width = near_dist * sin(field_of_view / 2.0f);
    float half_height = half_width / aspect_ratio;

    return (cam_data) {
        vector3_init(0, 0, 0), // position
        vector3_init(0, 0, near_dist), // look
        vector3_init(half_width, 0, 0), // right
        vector3_init(0, half_height, 0) // up
    };
}

void move_camera(cam_data * camera, vector4 vel) {
    camera->pos.x += vel.x;
    camera->pos.y += vel.y;
    camera->pos.z += vel.z;
    camera->pos.w += vel.w;
}

void rotate_camera_axis(cam_data * camera, vector4 axis, float theta) {
    // TODO
}

void rotate_camera(cam_data * camera, vector4 quat) {
    // TODO
}

void rotate_camera_yaw(cam_data * camera, float theta) {
    rotate_camera_axis(camera, camera->up, theta);
}

void rotate_camera_pitch(cam_data * camera, float theta) {
    rotate_camera_axis(camera, camera->right, theta);
}

void rotate_camera_roll(cam_data * camera, float theta) {
    rotate_camera_axis(camera, camera->look, theta);
}
