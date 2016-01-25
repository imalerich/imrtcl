//
//  camera.c
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/25/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include "camera.h"

void move_camera(struct cam_data * camera, struct vector4 vel, float delta) {
    camera->cam_pos.x += vel.x * delta;
    camera->cam_pos.y += vel.y * delta;
    camera->cam_pos.z += vel.z * delta;
    camera->cam_pos.w += vel.w * delta;
}