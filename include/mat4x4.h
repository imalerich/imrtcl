//
//  mat4x4.h
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/26/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef MAT4X4_H
#define MAT4X4_H

#include "vector.h"

typedef struct {
    vector4 x;
    vector4 y;
    vector4 z;
    vector4 w;
} mat4x4;

/**
 Initializes a matrix to the identity matrix.
 */
mat4x4 mat_identity();

/**
 Initializes a matrix with the contents of 
 'values' array, values is expected to be
 an array of length 16.
 \param values Initialization array for the matrix.
 \Return The matrix representation of 'values'.
 */
mat4x4 mat_init(float * f);

/**
 Generates a new matrix as the result of the product of a * b.
 \param a First operand of the matrix multiplication.
 \param b Second operand of the matrix multiplication.
 \return The result of the matrix multiplication.
 */
mat4x4 mat_multiply(mat4x4 a, mat4x4 b);

#endif