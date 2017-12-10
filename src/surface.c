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
		SURFACE_PLANE, pos.x, pos.y, pos.z, 1.0f,
		norm.x, norm.y, norm.z, 0.0f
	};

	memcpy(data, cpy, sizeof(cpy));
}

void make_triangle(vector4 p1, vector4 p2, vector4 p3, cl_float * data) {
	const cl_float cpy[] = {
		SURFACE_TRIANGLE,
		p1.x, p1.y, p1.z, 1.0,
		p2.x, p2.y, p2.z, 1.0,
		p3.x, p3.y, p3.z, 1.0
	};

	memcpy(data, cpy, sizeof(cpy));
}
