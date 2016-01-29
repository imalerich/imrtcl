//
//  surface.c
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/28/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include "surface.h"

surface make_sphere(vector4 pos, float radius) {
    return (surface) {
        vector4_init(pos.x, pos.y, pos.z, radius),
        zero_vector4(),
        SURFACE_SPHERE
    };
}

surface make_plane(vector4 pos, vector4 norm) {
    return (surface) {
        pos,
        norm,
        SURFACE_PLANE
    };
}

surface make_aa_cube(vector4 min, vector4 max) {
    return (surface) {
        min,
        max,
        SURFACE_AA_CUBE
    };
}