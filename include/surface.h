//
//  Created by Ian Malerich on 1/28/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef SURFACE_H
#define SURFACE_H

#include "vector.h"
#include "cl_util.h"

#define SURFACE_SPHERE  0.0f
#define SURFACE_PLANE   1.0f
#define SURFACE_AA_CUBE 2.0f

#define SPHERE_SIZE 5
#define PLANE_SIZE 7
#define AA_CUBE_SIZE 7

void make_sphere(vector4 pos, float radius, cl_float * data);

void make_plane(vector4 pos, vector4 norm, cl_float * data);

void make_aa_cube(vector4 min, vector4 max, cl_float * data);

#endif
