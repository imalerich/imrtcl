#define EPSILON 0.001
#define AMBIENT (0.0f/255.0f)

#define SURFACE_SPHERE  	0.0f
#define SURFACE_PLANE   	1.0f
#define SURFACE_TRIANGLE	2.0f

#define SPHERE_SIZE 5
#define PLANE_SIZE 9
#define TRIANGLE_SIZE 13

typedef struct {
    float4 diffuse;

    float spec_scalar;
    float spec_power;

    float reflect;
    float refract;
} material;

/* --------------------
 * Function Prototypes.
 * -------------------- */

float4 color_for_ray(float8 ray, float4 light_pos, __read_only __local float * surfaces, __local material * materials,
		int n_surfaces, int * hit_index, float4 * intersect, float4 * norm, uint * seed);
int intersect_ray_surfaces(float8 ray, __read_only __local float * surfaces,
		int n_surfaces, float4 * intersect, float4 * norm);
int size_of_surface(__read_only __local float * surface_p);

bool intersect_ray_surface(float8 ray, __read_only __local float * surface_p, float4 * intersect, float4 * norm);
bool intersect_ray_sphere(float8 ray, float4 sphere, float4 * intersect, float4 * norm);
bool intersect_ray_plane(float8 ray, float8 plane, float4 * intersect, float4 * norm);
bool intersect_ray_triangle(float8 ray, float4 p1, float4 p2, float4 p3, float4 * intersect, float4 * norm);

float8 calculate_ray(float4 camera_pos, float4 camera_look,
        float4 camera_right, float4 camera_up);
float scalar_for_lighting(float4 l_dir, float4 norm);
float specular_for_lighting(float8 ray, float4 l_dir, float4 norm, material mat);
float4 point_on_sphere(float4 sphere, uint * seed);
float4 bary_centric(float4 P, float4 p1, float4 p2, float4 p3);
uint rand(uint * seed);

/* --------------------
 * Kernel.
 * -------------------- */

__kernel void ray_tracer(
        // camera information
        float4 camera_pos,
        float4 camera_look,
        float4 camera_right,
        float4 camera_up,

        // seed for the random number generator
        uint random_seed,

		// scene information
		float4 light_pos,
		__read_only __global float * g_surfaces,
        __read_only __global material * g_materials,
		__local float * surfaces,
		__local material * materials,
		int n_surfaces, int n_surf_vals, int n_mat_vals,

        // kernel output
        __write_only image2d_t output
	) {

	event_t es = async_work_group_copy(surfaces, g_surfaces, n_surf_vals, 0);
	event_t em = async_work_group_copy((__local float *)materials, (__global float *)g_materials, n_mat_vals, 0);
	wait_group_events(1, &es);
	wait_group_events(1, &em);

    // the output image resolution -> global work size
	int screen_w = get_global_size(0);
	int screen_h = get_global_size(1);

    // the local (x, y) coordinate described relative to the global work size
	int x_pos = get_global_id(0);
	int y_pos = get_global_id(1);

	uint seed = random_seed;
	int hit_index;
    float8 ray = calculate_ray(camera_pos, camera_look, camera_right, camera_up);
	float4 intersect, norm; // surface intersection information

    float reflect = 1.0f; // percentage of color to use
    float4 color = (float4)0.0f; // sum of all color samples

    // grab all of our lighting samples
    for (int i = 0; i < 2; i++) {
		float4 c = color_for_ray(ray, light_pos, surfaces, materials, n_surfaces, 
				&hit_index, &intersect, &norm, &seed);
		
        // update the ray
        float r = 2.0f * dot(ray.hi, norm);
        ray = (float8){intersect, ray.hi - norm * r};

        // apply reflection
        if (hit_index >= 0 && materials[hit_index].reflect > EPSILON) {
            color += c * (1.0f - materials[hit_index].reflect) * reflect;
            reflect = materials[hit_index].reflect;
        } else {
            color += c * reflect;
            break;
        }
	}

	write_imagef(output, (int2)(x_pos, y_pos), color);
}

float4 color_for_ray(
		float8 ray,
		float4 light_pos,
		__read_only __local float * surfaces,
		__read_only __local material * materials,
		int n_surfaces,
		int * hit_index,
		float4 * intersect,
		float4 * norm,
		uint * seed) {

	 *hit_index = intersect_ray_surfaces(ray, surfaces, n_surfaces, intersect, norm);

	 float4 light_intersect, light_norm;
	 if (light_pos.w > EPSILON && intersect_ray_sphere(ray, light_pos, &light_intersect, &light_norm)) {
	     float pd = length(*intersect - ray.lo);
	     float ld = length(light_intersect - ray.lo);
	     if (ld <= pd) { return (float4)1.0f; }
	 }

	 if (*hit_index >= 0) {
	 	float diff = 0.0;
	 	float spec = 0.0;

        // check if the light is visible from this point
        int l_samples = light_pos.w > EPSILON ? 32 : 1;
	 	for (int l = 0; l < l_samples; l++) {
			float4 sample_pos = point_on_sphere(light_pos, seed);

	    	float l_dist = length(sample_pos - *intersect);
	    	float4 l_dir = normalize(sample_pos - *intersect);
	    	float sample_d = AMBIENT;
	    	float sample_s = 0.0f;

        	float4 l_int, l_norm;
        	if (intersect_ray_surfaces((float8)(*intersect, l_dir), 
						surfaces, n_surfaces, &l_int, &l_norm) < 0 
					|| length(*intersect - l_int) > l_dist) {
				float intensity = max((15.0f - l_dist)/15.0f, 0.0f);

            	// calculate the lighting components for this point
                sample_d = max(intensity * scalar_for_lighting(l_dir, *norm), sample_d);
                sample_s = max(intensity * specular_for_lighting(ray, l_dir, *norm, materials[*hit_index]), sample_s);
            }
	 	 		
             // add this samples contribution to the overall lighting
        	diff += sample_d / (float)l_samples;
        	spec += sample_s / (float)l_samples;
	 	}

        return (float4)((float3)diff, 1.0) * materials[*hit_index].diffuse +
			(float4)(1.0f, 1.0f, 1.0f, 0.0f) * max(spec * diff, 0.0f);
	 }

	 return (float4)0.0f;
}


/* --------------------
 * Ray Tests.
 * -------------------- */

int intersect_ray_surfaces(float8 ray, __read_only __local float * surfaces,
		int n_surfaces, float4 * intersect, float4 * norm) {

	 // information about the current nearest surfaces
	 int hit = -1;
	 float min_dist = -1;
	 float4 tmp_i, tmp_n;

	 __read_only __local float * tmp = surfaces;
	 for (int i=0; i<n_surfaces; i++) {
		if (intersect_ray_surface(ray, tmp, &tmp_i, &tmp_n)) {
	 		float dist = length(tmp_i - ray.lo);

	 		if (min_dist == -1 || dist < min_dist) {
	 			*intersect = tmp_i;
	 			*norm = tmp_n;

	 			hit = i;
	 			min_dist = dist;
	 		}
	 	}

	 	tmp += size_of_surface(tmp);
	 }

	 return hit;
}

int size_of_surface(__read_only __local float * surface_p) {
	const float surface_id = *surface_p;

	if (fabs(surface_id - SURFACE_SPHERE) < EPSILON) {
		return SPHERE_SIZE;
	} else if (fabs(surface_id - SURFACE_PLANE) < EPSILON) {
		return PLANE_SIZE;
	} else if (fabs(surface_id - SURFACE_TRIANGLE) < EPSILON) {
		return TRIANGLE_SIZE;
	} 
	
	return 0;
}

bool intersect_ray_surface(float8 ray, __read_only __local float * surface_p, float4 * intersect, float4 * norm) {
	const float surface_id = surface_p[0];
	++surface_p;

	if (fabs(surface_id - SURFACE_SPHERE) < EPSILON) {
		float4 sphere = (float4)(surface_p[0], surface_p[1], surface_p[2], surface_p[3]);
	 	return intersect_ray_sphere(ray, sphere, intersect, norm);

	} else if (fabs(surface_id - SURFACE_PLANE) < EPSILON) {
		float8 plane = (float8)(surface_p[0], surface_p[1], surface_p[2], surface_p[3],
		 		surface_p[4], surface_p[5], surface_p[6], surface_p[7]);
	 	return intersect_ray_plane(ray, plane, intersect, norm);

	} else if (fabs(surface_id - SURFACE_TRIANGLE) < EPSILON) {
		float4 p1 = (float4)(surface_p[0], surface_p[1], surface_p[2], surface_p[3]);
		float4 p2 = (float4)(surface_p[4], surface_p[5], surface_p[6], surface_p[7]);
		float4 p3 = (float4)(surface_p[8], surface_p[9], surface_p[10], surface_p[11]);
		if (!intersect_ray_triangle(ray, p1, p2, p3, intersect, norm)) {
			return intersect_ray_triangle(ray, p1, p3, p2, intersect, norm);
		} else { return true; }

	}

	return false;
}

/**
 * @brief Check if the input ray intersects the given sphere.
 * @param ray (input) Includes positional and directional information.
 * @param sphere (input) { pos.x, pos.y, pos.z, radius }
 * @param intersect (output) Intersection coordinate.
 * @param norm (output) Intersection normal vector.
 */
bool intersect_ray_sphere(float8 ray, float4 sphere, float4 * intersect, float4 * norm) {
    float4 center = {sphere.xyz, 0.0f};
    float radius = sphere.w;

    float a = pow(length(ray.hi), 2);
    float b = 2.0f * (dot(ray.lo, ray.hi) - dot(ray.hi, center));
    float c = pow(length(center - ray.lo), 2) - pow(radius, 2);
    float delta = pow(b, 2) - (4 * a * c);

    if (delta < -EPSILON || a == 0) {
        return 0;
    } else {
        float d = -(b + sqrt(delta)) / (2 * a);

        if (fabs(delta) > EPSILON) {
            float d0 = (-b - sqrt(delta)) / (2 * a);
            float d1 = (-b + sqrt(delta)) / (2 * a);

            d = (d0 < EPSILON) || (d1 < EPSILON) ? max(d0, d1) : min(d0, d1);
        }

        if (d > EPSILON) {
            if (intersect) {
                (*intersect) = ray.lo + d * ray.hi;
            }

            if (norm) {
                float4 i = ray.lo + d * ray.hi;
                (*norm) = normalize(i - center);
            }

            return true;
        } else {
            return false;
        }
    }
}

bool intersect_ray_plane(float8 ray, float8 plane, float4 * intersect, float4 * norm) {
	ray.hi = normalize(ray.hi);
	plane.hi = normalize(plane.hi);

	// the plane and the ray are parallel
	if (-EPSILON < dot(ray.hi, plane.hi) < EPSILON) {
		return false;
	}

	float n = dot(plane.lo - ray.lo, plane.hi);
	float d = n / dot(ray.hi, plane.hi);

	if (d > EPSILON) {
		(*intersect) = ray.lo + ray.hi * d;
		(*norm) = -plane.hi;
		return true;
	}

	return false;
}

bool intersect_ray_triangle(float8 ray, 
		float4 p1, float4 p2, float4 p3, 
		float4 * intersect, float4 * norm) {
	ray.hi = normalize(ray.hi);

	// normal of the plane on which this triagle lies
	float4 N = normalize(cross((p2 - p1), (p3 - p1)));

	// the plane and the ray are parallel
	if (-EPSILON < dot(ray.hi, N) < EPSILON) {
		return false;
	}

	// calculate where the plane intersects the ray
	float n = dot(p1 - ray.lo, N);
	float d = n / dot(ray.hi, N);
	if (d < EPSILON) { return false; }
	float4 P = ray.lo + ray.hi * d;

	// an intersection occurs if P lies on the input triangle
	float4 bary = bary_centric(P, p1, p2, p3);

	// if all barycentric coordinates are nonnegitave
	// than the point lies on the triangle
	if (bary.x >= 0.0 && bary.y >= 0.0 && bary.z >= 0.0) {
		(*intersect) = P;
		(*norm) = -N;
		return true;
	}

	return false;
}

/* --------------------
 * Utility Functions.
 * -------------------- */

/**
 * @brief
 * Given the input camera information, 
 * calculate the ray to shoot for the current work item.
 * @return Two concatenated vectors.
 * 	The first 4 floats describe vector position (the camera position).
 * 	The second 4 floats describe vector direction.
 */
float8 calculate_ray (
        float4 camera_pos,
        float4 camera_look,
        float4 camera_right,
        float4 camera_up
    ) {

    // the output image resolution -> global work size
    int screen_w = get_global_size(0);
    int screen_h = get_global_size(1);

    // the local (x, y) coordinate described relative to the global work size
    int x_pos = get_global_id(0);
    int y_pos = get_global_id(1);

    // calculate our coordinate as a percentage
    float x_perc = (x_pos / (float)screen_w) - 0.5;
    float y_perc = (y_pos / (float)screen_h) - 0.5;

    // compute the ray we will use for this work item
    float4 dir = normalize(camera_look + camera_up * -y_perc + camera_right * x_perc);
    return (float8){ camera_pos.xyz, 0.0, dir };
}

float scalar_for_lighting(float4 l_dir, float4 norm) {
    return max(min(dot(l_dir, normalize(norm)), 1.0f), 0.0f);
}

float specular_for_lighting(float8 ray, float4 l_dir, float4 norm, material mat) {
    // Blinn-Phong
    float4 b_dir = normalize(l_dir - ray.hi);
    float blinn = dot(b_dir, norm);
    return mat.spec_scalar * pow(blinn, mat.spec_power);
}

float4 point_on_sphere(float4 sphere, uint * seed) {
    float mil = 100000.0f;
    float4 r = (float4){
        (rand(seed) % (int)mil)/mil - 0.5,
        (rand(seed) % (int)mil)/mil - 0.5,
        (rand(seed) % (int)mil)/mil - 0.5,
        0.0
    };

    return (normalize(r) * (float4)sphere.w) + (float4){sphere.xyz, 1.0};
}

float4 bary_centric(float4 P, float4 p1, float4 p2, float4 p3) {
	float4 R = P - p1;
	float4 Q1 = p2 - p1;
	float4 Q2 = p3 - p1;

	// a, b, c, d form a 2x2 matrix
	// OpenCL doesn't have built in matrix operations
	// apparently, but since our system is small
	// doing the work by hand isn't too bad
	float a = dot(Q1, Q1);
	float b = dot(Q1, Q2);
	float c = b;
	float d = dot(Q2, Q2);

	float det = (a*d) - (b*c);
	if (det == 0) { return (float4)(-1.0, -1.0, -1.0, -1.0); }

	float2 RQ = (float2)(dot(R, Q1), dot(R, Q2));
	float w2 = dot((float2)(d/det, -b/det), RQ);
	float w3 = dot((float2)(-c/det, a/det), RQ);
	float w1 = 1 - w2 - w3;
	return (float4)(w1, w2, w3, 0.0);
}

uint rand(uint * seed) {
	int screen_w = get_global_size(0);
	int x_pos = get_global_id(0);
	int y_pos = get_global_id(1);
	int id = screen_w * y_pos + x_pos;

	*seed = *seed + id;
	*seed = (*seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
	*seed = *seed >> 16;

	return *seed;
}

