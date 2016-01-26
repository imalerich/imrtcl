//
//  vector.c
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/18/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include <math.h>
#include "vector.h"

struct vector4 vector4() {
    return (struct vector4) { 0.0f, 0.0f, 0.0f, 0.0f };
}

struct vector4 quaternion() {
    return (struct vector4) { 1.0f, 0.0f, 0.0f, 0.0f };
}

struct vector4 vector3_init(float x, float y, float z) {
    return (struct vector4) { x, y, z, 0.0 };
}

struct vector4 vector4_init(float x, float y, float z, float w) {
    return (struct vector4) { x, y, z, w };
}

struct vector4 vector4_quaternion(struct vector4 axis, float theta) {
    return vector4_init(
                        axis.x * sinf(theta / 2.0f),
                        axis.y * sinf(theta / 2.0f),
                        axis.z * sinf(theta / 2.0f),
                        cosf(theta / 2.0f)
                        );
}

struct vector4 vector4_mult(struct vector4 a, struct vector4 b) {
    return vector4_init(
                        a.x * b.x,
                        a.y * b.y,
                        a.z * b.z,
                        a.w * b.w
                        );
}

struct vector4 quaternion_mult(struct vector4 a, struct vector4 b) {
    return vector4_init(
                        (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y),
                        (a.w * b.y) - (a.x * b.z) + (a.y * b.w) + (a.z * b.x),
                        (a.w * b.z) + (a.x * b.y) - (a.y * b.x) + (a.z * b.w),
                        (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z)
                        );
}

struct vector4 cross3(struct vector4 a, struct vector4 b) {
    return vector3_init(
                   (a.y * b.z) - (a.z * b.y),
                   (a.z * b.x) - (a.x * b.z),
                   (a.x * b.y) - (a.y * b.x)
                   );
}

float dot(struct vector4 a, struct vector4 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

float length(struct vector4 a) {
    return sqrtf(pow(a.x, 2) + pow(a.y, 2) + pow(a.z, 2) + pow(a.w, 2));
}

struct vector4 normalize(struct vector4 a) {
    float l = length(a);
    return vector4_init(
                        a.x / l,
                        a.y / l,
                        a.z / l,
                        a.w / l
                        );
}