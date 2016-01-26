#define EPSILON 0.00000000001

// function prototypes
float8 calculate_ray (float4 camera_pos, float4 camera_look,
        float4 camera_right, float4 camera_up);
float4 point_on_sphere(float4 sphere, uint * seed);
float scalar_for_lighting(float8 ray, float4 l_dir, float4 norm);
uint rand(uint * seed);
int intersect_ray_surfaces(float8 ray, __global float4 * surfaces, int n, float4 * intersect, float4 * norm);
int intersect_ray_any_surface(float8 ray, __global float4 * surfaces, int n);
bool intersect_ray_sphere(float8 ray, float4 sphere, float4 * intersect, float4 * norm);

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
        __global float4 * surfaces,
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

    float4 norm, intersect;
    int hit = intersect_ray_surfaces(ray, surfaces, n_surfaces, &intersect, &norm);

    if (hit >= 0) {
        float d = 0.0;

        // check if the light is visible from this point
        int l_samples = light_pos.w > 0.0 ? 32 : 1;
        for (int l = 0; l < l_samples; l++) {
            float4 sample_pos = point_on_sphere(light_pos, &seed);

            float4 l_dir = normalize(sample_pos - intersect);
            float sample_d = 10.0f/255.0f;

            if (intersect_ray_any_surface((float8){intersect, l_dir}, surfaces, n_surfaces) < 0) {
                sample_d = max(scalar_for_lighting(ray, l_dir, norm), 10/255.0f);
            }

            // add this samples contribution to the overall lighting
            d += sample_d / (float)l_samples;
        }

        write_imagef(output, (int2){x_pos, y_pos}, (float4)(d, d, d, 1.0));
    } else {
        write_imagef(output, (int2){x_pos, y_pos}, (float4){49/255.0, 51/255.0, 71/255.0, 1.0});
    }
}

/*
 Calculates a lighting sample for the given normal and light direction.
 The function will then return a scalar in the range of 0.0f -> 1.0f
 for the given point.
 */
float scalar_for_lighting(float8 ray, float4 l_dir, float4 norm) {
    norm = normalize(norm);
    float l = dot(l_dir, norm);

    // Blinn-Phong
    float4 b_dir = normalize(l_dir - ray.hi);
    float blinn = dot(b_dir, norm);
    float spec = 0.5f * pow(blinn, 64.0f);

    return max(min(l + spec, 1.0f), 0.0f);
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
int intersect_ray_surfaces(float8 ray, __global float4 * surfaces, int n,
                           float4 * intersect, float4 * norm) {
    int hit = -1;
    float min_dist = -1;
    float4 tmp_i, tmp_n;

    // for each surface
    for (int i = 0; i < n; i++) {
        // check if the input ray hits the input surface
        if (intersect_ray_sphere(ray, surfaces[i], &tmp_i, &tmp_n)) {
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

/*
 Performs a similar intersection test to intersect_ray_surfaces(...),
 however this function will return immediately as soon as an intersection
 occurs. Use this when you need to know if an intersectino occurs, but 
 not the which surfaces is nearest to the rays origin.
 */
int intersect_ray_any_surface(float8 ray, __global float4 * surfaces, int n) {
    // loop through each surface and try to find an intersection
    for (int i = 0; i < n; i++) {
        if (intersect_ray_sphere(ray, surfaces[i], 0, 0)) {
            return 1;
        }
    }

    return -1;
}

/*
 Determines whether or not the input ray intersects
 with the given input sphere.
 The low 4 components of the ray will be treated
 as the intial position, while the upper 4 componenents
 will be used as the rays direction vector.
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

        if (d > EPSILON) {
            float d0 = (-b - sqrt(delta)) / (2 * a);
            float d1 = (-b + sqrt(delta)) / (2 * a);

            d = (d0 < EPSILON) || (d1 < EPSILON) ? max(d0, d1) : min(d0, d1);
        }

        if (d > EPSILON) {
            if (intersect && norm) {
                (*intersect) = ray.lo + d * ray.hi;
                (*norm) = normalize(*intersect - center);
            }

            return 1;
        } else {
            return 0;
        }
    }
}
