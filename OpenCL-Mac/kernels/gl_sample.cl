__kernel void gl_sample(
        float delta,
        __write_only image2d_t output
	) {

	int screen_w = get_global_size(0);
	int screen_h = get_global_size(1);

	int x_pos = get_global_id(0);
	int y_pos = get_global_id(1);
    int2 coords = (int2){x_pos, y_pos};

	float4 val = (float4)(delta * x_pos/(float)screen_w,
                          delta * y_pos/(float)screen_h,
                          delta,
                          1.0);
    write_imagef(output, coords, val);
}
