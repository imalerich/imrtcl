#define EPSILON 0.00000000001

// function prototypes
float8 calculate_ray (float4 camera_pos, float4 camera_look,
        float4 camera_right, float4 camera_up);
bool intersect_ray_sphere(float8 ray, float4 sphere, float4 * intersect, float4 * norm);

/* ---------------------------------------------------
 Generates an image buffer by ray tracing each pixel
 for the scene given as parameters to this kernel.
 
 The camera_up and camera_right vectors should have a
 magnitude corresponding to half the width of the axis
 of the image they describe.
 
 In the current implementation surfaces may only be
 spheres each contained within a single float4.
 Components .xyz represent the center coordinate while
 the .w component represent the radius of the sphere.
 --------------------------------------------------- */
__kernel void ray_tracer(
        float4 camera_pos,
        float4 camera_look,
        float4 camera_right,
        float4 camera_up,

        float4 light_pos,

        __global float4 * surfaces,
        int n_surfaces,
        write_only image2d_t output
    ) {

    // the output image resolution -> global work size
    int screen_w = get_global_size(0);
    int screen_h = get_global_size(1);

    // the local (x, y) coordinate described relative to the global work size
    int x_pos = get_global_id(0);
    int y_pos = get_global_id(1);

    float8 ray = calculate_ray(camera_pos, camera_look, camera_right, camera_up);

    bool hit = 0;
    float4 norm, intersect;
    for (int i = 0; i < n_surfaces; i++) {
        if (intersect_ray_sphere(ray, surfaces[i], &intersect, &norm)) {
            hit = 1;
            break;
        }
    }

    if (hit) {
        float4 l_dir = normalize(light_pos - intersect);
        float d = max(dot(l_dir, norm), 10/255.0f);

        write_imagef(output, (int2){x_pos, y_pos}, (float4)(d, d, d, 1.0));
    } else {
        write_imagef(output, (int2){x_pos, y_pos}, (float4){49/255.0, 51/255.0, 71/255.0, 1.0});
    }
}

/* ---------------------------------------------------
 Takes the input camera vectors and calculates the
 ray that will be used for a particular work item.
 --------------------------------------------------- */
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

/* ---------------------------------------------------
 Determines whether or not the input ray intersects
 with the given input sphere.
 The low 4 components of the ray will be treated
 as the intial position, while the upper 4 componenents
 will be used as the rays direction vector.
 The spheres position in space will be treated as
 its .xyz components while its .w component will
 be used for the radius of the sphere.
 --------------------------------------------------- */
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

        if (delta > EPSILON) {
            (*intersect) = ray.lo + d * ray.hi;
            (*norm) = normalize(*intersect - center);

            return 1;
        } else {
            return 0;
        }
    }
}
