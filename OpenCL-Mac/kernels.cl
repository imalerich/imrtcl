__kernel void square(__global float * input, __global float * output, const unsigned int count) {

    // takes a single element of 'input' and stores the square in 'output'
    int i = get_global_id(0);
    if (i < count) {
        output[i] = input[i] * input[i];
    }
}