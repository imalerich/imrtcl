//
//  Created by Ian Malerich on 1/28/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef SURFACE_H
#define SURFACE_H

#include "vector.h"
#include "cl_util.h"

#define SURFACE_SPHERE  0
#define SURFACE_PLANE   1
#define SURFACE_AA_CUBE 2

/**
 Contains a representation of some shape.
 This struct will fit into the surface
 structure that is used by the ray tracer
 kernel.
 */
typedef struct __attribute__ ((packed)) {
    vector4 data_lo;
    vector4 data_hi;
    cl_int shape_id;
} surface;

/**
 Creates a surface representation of a sphere.
 \param pos .xyz coordinate of the sphere, .w is ignored.
 \param radius The radius of the input sphere.
 \return The surface representation.
 */
surface make_sphere(vector4 pos, float radius);

/**
 Creates a surface representation of a plane.
 \param pos A coordinate on the plane.
 \param norm Normal vector to the plane.
 \return The surface representation.
 */
surface make_plane(vector4 pos, vector4 norm);

/**
 Creates a surface representation of an axis-aligned cube.
 \param min Minimum coordinate of the cube.
 \param max Maximum coordinate of the cube.
 \return The surface representation.
 */
surface make_aa_cube(vector4 min, vector4 max);

#endif
