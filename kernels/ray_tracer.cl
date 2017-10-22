#define EPSILON 0.001
#define AMBIENT (0.0f/255.0f)

#define SURFACE_SPHERE 0.0f
#define SURFACE_PLANE 1.0f

#define SPHERE_SIZE 5
#define PLANE_SIZE 9

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

float4 color_for_ray(float8 ray, float4 light_pos, __global float * surfaces, __global material * materials,
		int n_surfaces, int * hit_index, float4 * intersect, float4 * norm, uint * seed);
int intersect_ray_surfaces(float8 ray, __global float * surfaces,
		int n_surfaces, float4 * intersect, float4 * norm);
int size_of_surface(__global float * surface_p);

bool intersect_ray_surface(float8 ray, __global float * surface_p, float4 * intersect, float4 * norm);
bool intersect_ray_sphere(float8 ray, float4 sphere, float4 * intersect, float4 * norm);
bool intersect_ray_plane(float8 ray, float8 plane, float4 * intersect, float4 * norm);

float8 calculate_ray(float4 camera_pos, float4 camera_look,
        float4 camera_right, float4 camera_up);
float scalar_for_lighting(float4 l_dir, float4 norm);
float specular_for_lighting(float8 ray, float4 l_dir, float4 norm, material mat);
float4 point_on_sphere(float4 sphere, uint * seed);
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
        uint seed,

		// scene information
		float4 light_pos,
		__global float * surfaces,
        __global material * materials,
		int n_surfaces,

        // kernel output
        __write_only image2d_t output
	) {

    // the output image resolution -> global work size
	int screen_w = get_global_size(0);
	int screen_h = get_global_size(1);

    // the local (x, y) coordinate described relative to the global work size
	int x_pos = get_global_id(0);
	int y_pos = get_global_id(1);

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
		__global float * surfaces,
		__global material * materials,
		int n_surfaces,
		int * hit_index,
		float4 * intersect,
		float4 * norm,
		uint * seed) {

	 *hit_index = intersect_ray_surfaces(ray, surfaces, n_surfaces, intersect, norm);

	 if (*hit_index >= 0) {
	 	float diff = 0.0;
	 	float spec = 0.0;

        // check if the light is visible from this point
        int l_samples = light_pos.w > 0.0 ? 16 : 1;
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
				float intensity = max((10.0f - l_dist)/10.0f, 0.0f);

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

int intersect_ray_surfaces(float8 ray, __global float * surfaces,
		int n_surfaces, float4 * intersect, float4 * norm) {

	 // information about the current nearest surfaces
	 int hit = -1;
	 float min_dist = -1;
	 float4 tmp_i, tmp_n;

	 __global float * tmp = surfaces;
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

int size_of_surface(__global float * surface_p) {
	const float surface_id = *surface_p;

	if (fabs(surface_id - SURFACE_SPHERE) < EPSILON) {
		return SPHERE_SIZE;
	} else if (fabs(surface_id - SURFACE_PLANE) < EPSILON) {
		return PLANE_SIZE;
	}
	
	return 0;
}

bool intersect_ray_surface(float8 ray, __global float * surface_p, float4 * intersect, float4 * norm) {
	const float surface_id = surface_p[0];

	if (fabs(surface_id - SURFACE_SPHERE) < EPSILON) {
		//float4 sphere = *((__global float4 *)(surface_p+1));
		float4 sphere = (float4)(surface_p[1], surface_p[2], surface_p[3], surface_p[4]);
	 	return intersect_ray_sphere(ray, sphere, intersect, norm);

	} else if (fabs(surface_id - SURFACE_PLANE) < EPSILON) {
	 	// float8 plane = *((__global float8 *)surface_p);
		float8 plane = (float8)(surface_p[1], surface_p[2], surface_p[3], surface_p[4],
				surface_p[5], surface_p[6], surface_p[7], surface_p[8]);
	 	return intersect_ray_plane(ray, plane, intersect, norm);
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
	if (abs(dot(ray.hi, plane.hi) < EPSILON)) {
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
    float mil = pow(10.0f, 6.0f);
    float4 r = (float4){
        (rand(seed) % (int)mil)/mil - 0.5,
        (rand(seed) % (int)mil)/mil - 0.5,
        (rand(seed) % (int)mil)/mil - 0.5,
        0.0
    };

    return normalize(r) * (float4)sphere.w + (float4){sphere.xyz, 0.0};
}

uint rand(uint * seed) {
	int screen_w = get_global_size(0);
	int x_pos = get_global_id(0);
	int y_pos = get_global_id(1);
	int id = screen_w * y_pos + x_pos;

	*seed = *seed + id;
	*seed = (*seed + 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
	*seed = *seed >> 16;

	return *seed;
}

