__kernel void gl_sample(
		__global float4 * output
	) {

	int screen_w = get_global_size(0);
	int screen_h = get_global_size(1);

	int x_pos = get_global_id(0);
	int y_pos = get_global_id(1);

	output[screen_w * y_pos + x_pos] = 
		(float4)(x_pos/(float)screen_w, y_pos/(float)screen_h, x_pos/(float)screen_w, 1.0);
}
