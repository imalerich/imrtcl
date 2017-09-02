#define EPSILON 0.001

#define SURFACE_SPHERE  0
#define SURFACE_PLANE   1
#define SURFACE_AA_CUBE 2

typedef struct __attribute__ ((packed)) {
    float8 data;    // all data should be packed into this component
    int shape_id;   // ie. SHAPE_SPHERE
} surface;

typedef struct {
    float4 diffuse;

    float spec_scalar;
    float spec_power;

    float reflect;
    float refract;
} material;

// function prototypes
float8 calculate_ray (float4 camera_pos, float4 camera_look,
        float4 camera_right, float4 camera_up);
float4 color_for_ray(float8 ray, float4 light_pos, __global surface * surfaces,
                     __global material * materials, int n_surfaces,
                     int * hit_index, float4 * intersect, float4 * norm, int * seed);
float4 point_on_sphere(float4 sphere, uint * seed);
float scalar_for_lighting(float4 l_dir, float4 norm);
float specular_for_lighting(float8 ray, float4 l_dir, float4 norm, material mat);
uint rand(uint * seed);
int intersect_ray_surfaces(float8 ray, __global surface * surfaces, int n, float4 * intersect, float4 * norm);
bool intersect_ray_surface(float8 ray, surface surface, float4 * intersect, float4 * norm);
bool intersect_ray_sphere(float8 ray, float4 sphere, float4 * intersect, float4 * norm);
bool intersect_ray_plane(float8 ray, float8 plane, float4 * intersect, float4 * norm);
bool intersect_ray_aa_cube(float8 ray, float8 cube, float4 * intersect, float4 * norm);

/*
 Generates an image buffer by ray tracing each pixel
 for the scene given as parameters to this kernel.
 
 The camera_up and camera_right vectors should have a
 magnitude corresponding to half the width of the axis
 of the image they describe.
 
 In the current implementation surfaces may only be
 spheres each contained within a single float4.
 Components .xyz represent the center coordinate while
 the .w component represent the radius of the sphere.
 */
__kernel void ray_tracer(
        // camera information
        float4 camera_pos,
        float4 camera_look,
        float4 camera_right,
        float4 camera_up,

        // scene information
        float4 light_pos,
        __global surface * surfaces,
        __global material * materials,
        int n_surfaces,

        // seed for the random number generator
        int seed,

        // kernel output
        __write_only image2d_t output
    ) {

    // the output image resolution -> global work size
    int screen_w = get_global_size(0);
    int screen_h = get_global_size(1);

    // the local (x, y) coordinate described relative to the global work size
    int x_pos = get_global_id(0);
    int y_pos = get_global_id(1);

    float8 ray = calculate_ray(camera_pos, camera_look, camera_right, camera_up);

    int hit_index;
    float4 norm, intersect; // surface intersection information
    float reflect = 1.0f; // percentage of color to use
    float4 color = (float4)0.0f; // sum of all color samples

    // grab all of our lighting samples
    for (int i = 0; i < 5; i++) {
        // get the current color
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

    write_imagef(output, (int2){x_pos, y_pos}, color);
}

/*
 Computes the color component for the input 'ray'.
 */
float4 color_for_ray(
                     float8 ray,
                     float4 light_pos,
                     __global surface * surfaces,
                     __global material * materials,
                     int n_surfaces,
                     int * hit_index,
                     float4 * intersect,
                     float4 * norm,
                     int * seed
                     ) {

    *hit_index = intersect_ray_surfaces(ray, surfaces, n_surfaces, intersect, norm);

    float4 light_intersect;
    if (intersect_ray_sphere(ray, light_pos, &light_intersect, 0)) {
        float pd = length(*intersect - ray.lo);
        float ld = length(light_intersect - ray.lo);

        if (ld <= pd) {
            return (float4)1.0f;
        }
    }

    if (*hit_index >= 0) {
        float diff = 0.0;
        float spec = 0.0;

        // check if the light is visible from this point
        int l_samples = light_pos.w > 0.0 ? 128 : 1;
        for (int l = 0; l < l_samples; l++) {
            float4 sample_pos = point_on_sphere(light_pos, seed);

            float l_dist = length(sample_pos - *intersect);
            float4 l_dir = normalize(sample_pos - *intersect);
            float sample_d = 10.0f/255.0f;
            float sample_s = 0.0f;

            float4 l_int;
            if (intersect_ray_surfaces((float8){*intersect, l_dir},
                                       surfaces, n_surfaces, &l_int, 0) < 0
                                       || length(*intersect - l_int) > l_dist) {
                // calculate the lighting compontnets for this point
                sample_d = max(scalar_for_lighting(l_dir, *norm), sample_d);
                sample_s = max(specular_for_lighting(ray, l_dir, *norm,
                                                   materials[*hit_index]), sample_s);
            }

            // add this samples contribution to the overall lighting
            diff += sample_d / (float)l_samples;
            spec += sample_s / (float)l_samples;
        }

        return (float4)((float3)diff, 1.0) * materials[*hit_index].diffuse +
        (float4)(1.0f, 1.0f, 1.0f, 0.0f) * max(spec * diff, 0.0f);
    } else {
        return (float4)0.0f;
    }
}

/*
 Calculates a lighting sample for the given normal and light direction.
 The function will then return a scalar in the range of 0.0f -> 1.0f
 for the given point.
 */
float scalar_for_lighting(float4 l_dir, float4 norm) {
    return max(min(dot(l_dir, normalize(norm)), 1.0f), 0.0f);
}

/*
 Calculates the specular component for a given point using
 the Blinn-Phong method.
 */
float specular_for_lighting(float8 ray, float4 l_dir, float4 norm, material mat) {
    // Blinn-Phong
    float4 b_dir = normalize(l_dir - ray.hi);
    float blinn = dot(b_dir, norm);
    return mat.spec_scalar * pow(blinn, mat.spec_power);
}

/*
 Returns a random point on the input sphere, using the given
 random number seed. This function will push a new seed into
 the provided seed upon completion.
 */
float4 point_on_sphere(float4 sphere, uint * seed) {
    int mil = 100000;
    float4 r = (float4){
        (rand(seed) % mil)/(float)mil - 0.5,
        (rand(seed) % mil)/(float)mil - 0.5,
        (rand(seed) % mil)/(float)mil - 0.5,
        0.0
    };

    return normalize(r) * (float4)sphere.w + (float4){sphere.xyz, 0.0};
}

/*
 Takes the input camera vectors and calculates the
 ray that will be used for a particular work item.
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

/*
 Implements a basic random number generatoed. Given
 an input seed the next random value in the sequence 
 will be generated.
 */
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

/*
 Performs ray intersections tests across the given scene. The object
 whos intersection occurs nearest to the origin of the ray
 is returned as an index into the surfaces array. If an intersection 
 does not occur -1 will be returned.
 */
int intersect_ray_surfaces(float8 ray, __global surface * surfaces, int n,
                           float4 * intersect, float4 * norm) {
    int hit = -1;
    float min_dist = -1;
    float4 tmp_i, tmp_n;

    // for each surface
    for (int i = 0; i < n; i++) {
        // check if the input ray hits the input surface
        if (intersect_ray_surface(ray, surfaces[i], &tmp_i, &tmp_n)) {
            float dist = length(tmp_i - ray.lo);

            // ignore the intersection if it is not closer than the last
            if (min_dist == -1 || dist < min_dist) {
                // update the intersection and normal coordinates
                *intersect = tmp_i;
                *norm = tmp_n;

                hit = i;
                min_dist = dist;
            }
        }
    }

    return hit;
}

/**
 Redirects a ray intersection test to the proper function for 
 the input surface type.
 */
bool intersect_ray_surface(float8 ray, surface surface, float4 * intersect, float4 * norm) {
    switch (surface.shape_id) {
        case SURFACE_SPHERE:
            return intersect_ray_sphere(ray, surface.data.lo, intersect, norm);

        case SURFACE_PLANE:
            return intersect_ray_plane(ray, surface.data, intersect, norm);

        case SURFACE_AA_CUBE:
            return intersect_ray_aa_cube(ray, surface.data, intersect, norm);

        default:
            return 0;
    }
}

/*
 Determines whether or not the input ray intersects
 with the given input sphere.
 The spheres position in space will be treated as
 its .xyz components while its .w component will
 be used for the radius of the sphere.
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

            return 1;
        } else {
            return 0;
        }
    }
}

/*
 Determines whether or not the input ray
 intersects with the given input plane.
 Plane.lo will be treated as a point on 
 the input plane, while Plane.hi will serve
 as the normal vector for the plane.
 */
bool intersect_ray_plane(float8 ray, float8 plane, float4 * intersect, float4 * norm) {
    // the plane and the ray are parallel
    if (fabs(dot(ray.hi, plane.hi)) < EPSILON) {
        return 0;
    }

    float n = dot(plane.lo - ray.lo, plane.hi);
    float d = n / dot(ray.hi, plane.hi);

    if (d > EPSILON) {
        (*intersect) = ray.lo + ray.hi * d;
        (*norm) = plane.hi;
        return 1;
    } else {
        return 0;
    }
}

/*
 Determines whether or not the input ray
 intersects with the given input aa-cube.
 Cube.lo will be treated as the minimum position
 of the cube, representing the minimu values
 on each cardinal axis. While cube.hi will
 be treated as just the opposite, as the maximum
 position of the cube.
 */
bool intersect_ray_aa_cube(float8 ray, float8 cube, float4 * intersect, float4 * norm) {
    return false;
}