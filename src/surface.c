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
