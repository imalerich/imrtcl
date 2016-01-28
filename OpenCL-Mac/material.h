//
//  material.h
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/27/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef MATERIAL_H
#define MATERIAL_H

#include "vector.h"

/**
 Contains all necessary data that the kernel will
 need for rendering a material. The format of this
 struct aligns with the 'material' struct in the 
 OpenCL code for the ray tracer.
 */
typedef struct {
    vector4 diffuse;

    float spec_scalar;
    float spec_power;

    float reflect;
    float refract;
} material;

/**
 Utility method that generates a random material.
 */
material rand_material();

/**
 Utility to generate a simple material with only
 a diffuse componenent.
 */
material diffuse_material(vector4 diffuse);

#endif
