__kernel void gl_sample(
        __write_only image2d_t output
	) {

	int screen_w = get_global_size(0);
	int screen_h = get_global_size(1);

	int x_pos = get_global_id(0);
	int y_pos = get_global_id(1);

	float4 val = (float4){x_pos/(float)screen_w,
                          y_pos/(float)screen_h,
                          1.0,
                          1.0};
    write_imagef(output, (int2){x_pos, y_pos}, val);
}
