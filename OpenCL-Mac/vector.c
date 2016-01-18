//
//  vector.c
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/18/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include "vector.h"

struct vector4 vector4() {
    return (struct vector4) {0.0f, 0.0f, 0.0f, 0.0f};
}

struct vector4 vector4_init(float x, float y, float z, float w) {
    return (struct vector4) {x, y, z, w};
}