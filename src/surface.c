//
//  Created by Ian Malerich on 1/28/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "surface.h"

void make_sphere(vector4 pos, cl_float radius, float * data) {
	const cl_float cpy[] = {
		SURFACE_SPHERE, pos.x, pos.y, pos.z, radius
	};

	memcpy(data, cpy, sizeof(cpy));
}

void make_plane(vector4 pos, vector4 norm, float * data) {
	const cl_float cpy[] = {
		SURFACE_PLANE, pos.x, pos.y, pos.z, 
		norm.x, norm.y, norm.z
	};

	memcpy(data, cpy, sizeof(cpy));
}

void make_aa_cube(vector4 min, vector4 max, float * data) {
	const cl_float cpy[] = {
		SURFACE_AA_CUBE, min.x, min.y, min.z,
		max.x, max.y, max.z
	};

	memcpy(data, cpy, sizeof(cpy));
}
