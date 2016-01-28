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
        rand_vector4(),
        (rand() % 1000) / 1000.0f, rand() % 32, // specular
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