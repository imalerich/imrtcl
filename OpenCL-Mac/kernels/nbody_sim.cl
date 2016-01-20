/* ---------------------------------------------------
 Performs an N-Body simulation on the input data set.
 The 4 dimmensional input vectors pos_old and pos_new
 will contain the {x, y, z} coordinates of the particle 
 they represent, as well as {w} representing the mass
 of the input particle.
 
 The parameter pblock points to local memory on the
 compute device and will be used to cache positions
 for reuse on later iterations.
 --------------------------------------------------- */
__kernel void nbody(
        float delta, float eps,
        __global float4 * pos_old,
        __global float4 * pos_new,
        __global float4 * cur_vel,
        __local float4 * pblock
    ) {

    const float4 v_delta = (float4){delta, delta, delta, 0.0};
    int i = get_global_id(0);
    int i_local = get_local_id(0);

    int n = get_global_size(0);
    int n_local = get_local_size(0);
    int n_blocks = n / n_local;

    float4 p = pos_old[i];
    float4 v = cur_vel[i];
    float4 a = (float4){0.0, 0.0, 0.0, 0.0};

    // for each block
    for (int j_block = 0; j_block < n_blocks; j_block++) {
        // cache one particle position
        pblock[i_local] = pos_old[j_block * n_local * i_local];

        // wait for the rest of the work group to finish
        barrier(CLK_LOCAL_MEM_FENCE);

        // now loop through all of the cached particles
        for (int j = 0; j < n_local; j++) {
            float4 p2 = pblock[j];
            float4 d = p2 - p;

            float invr = rsqrt(d.x * d.x + d.y * d.y + d.z * d.z + eps);
            float force = p2.w * (invr * invr * invr);

            // accumlate the accleration vector
            a += force * d;
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        // apply the acceleration
        p += v_delta * v + 0.5f * v_delta * v_delta * a;
        v += v_delta * a;

        // update the global data representation
        pos_new[i] = p;
        cur_vel[i] = v;
    }
}