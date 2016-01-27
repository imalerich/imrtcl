//
//  mat4x4.c
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/26/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include <stdlib.h>

#include "mat4x4.h"

struct mat4x4 mat_identity() {
    return (struct mat4x4) {
        vector4_init(1, 0, 0, 0),
        vector4_init(0, 1, 0, 0),
        vector4_init(0, 0, 1, 0),
        vector4_init(0, 0, 0, 1)
    };
}

struct mat4x4 mat_init(float * f) {
    return (struct mat4x4) {
        vector4_init(f[0], f[1], f[2], f[3]),
        vector4_init(f[4], f[5], f[6], f[7]),
        vector4_init(f[8], f[9], f[10], f[11]),
        vector4_init(f[12], f[13], f[14], f[15])
    };
}

struct mat4x4 mat_multiply(struct mat4x4 a, struct mat4x4 b) {
    float * m0 = (float *)&a.x.x;
    float * m1 = (float *)&b.x.x;
    float * p = (float *)malloc(sizeof(float) * 16);

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            // do stuff here
            int i = r * 4 + c;
            p[i] = m0[i] * m1[i];
        }
    }

    // convert the float pointer to a matrix and return the result
    struct mat4x4 tmp = mat_init(p);
    free(p);
    return tmp;
}