//
//  material.c
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/27/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include <stdlib.h>
#include "material.h"

material rand_material() {
    return (material) {
        rand_vector4(),             // diffuse
        (rand() % 1000) / 1000.0f,  // specular-scalar
        (float)(rand() % 32),  // specular-power
        (rand() % 1000) / 1000.0f,  // reflection
        (rand() % 1000) / 1000.0f   // refraction
    };
}

material diffuse_material(vector4 diffuse) {
    return (material) {
        diffuse,
        0.0f, 0.0f, // specular
        0.0f, 0.0f  // reflection and refraction
    };
}

material specular_material(vector4 diffuse, float spec_scalar, float spec_power) {
    return (material) {
        diffuse,
        spec_scalar, spec_power,
        0.0f, 0.0f  // reflection and refraction
    };
}
