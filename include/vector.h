//
//  Created by Ian Malerich on 1/18/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef vector_h
#define vector_h

#include "cl_util.h"

typedef struct {
    cl_float x;
    cl_float y;
    cl_float z;
    cl_float w;
} vector4;

/**
 Initialize a vector4 to { 0.0f, 0.0f, 0.0f, 0.0f }.
 */
vector4 zero_vector4();

/**
 Initializes a vector4 to { 1.0f, 0.0f, 0.0f, 0.0f }.
 This quaternion represents no rotation.
 */
vector4 quaternion();

/**
Initialize a vector4 to { x, y, z, 0.0f }.
 */
vector4 vector3_init(float x, float y, float z);

/**
 Initialize a vector4 to { x, y, z, w }.
 */
vector4 vector4_init(float x, float y, float z, float w);

/**
 Generates a random vector. 
 Each component will be within
 the range of (0.0f, 1.0f).
 \return The randomly generated vector.
 */
vector4 rand_vector4();

/**
 Constructs a quaternion vector from the input 'axis' (the w
 component of that vector will be ignored), and the input theta.
 \param axis Axis of rotation.
 \param theta Angle of rotation in radians.
 */
vector4 vector4_quaternion(vector4 axis, float theta);

/**
 Multiplies the two input vectors per component, the result is then
 returned.
 \param a First operand of multiplication.
 \param b Second operand of multiplication.
 \return The result of the multiplication.
 */
vector4 vector4_mult(vector4 a, vector4 b);

/**
 Multiplies two vectors representing quaternions, the resulting
 vector will represent the rotations of both quaternions.
 \param a Firt operand of multiplication.
 \param b Second operand of multiplication.
 \return A quaternion representing the rotations in 'a' and 'b'.
 */
vector4 quaternion_mult(vector4 a, vector4 b);

/**
 Vector cross product. For simplicity this method will ignore
 the 'w' components of each input vector, and treats the
 vectors as 3 dimmensional.
 \param a First operand of the dot product.
 \param b Second operand of the dot product.
 \return Vector result of the cross product.
 */
vector4 cross3(vector4 a, vector4 b);

/**
 Vector dot product.
 \param a First operand of the dot product.
 \param b Second operand of the dot product.
 \return Scalar result of the dot product.
 */
float dot(vector4 a, vector4 b);

/**
 Determines the length of the input vector.
 \param a The vector to find the length of.
 \return The length of the input vector.
 */
float length(vector4 a);

/**
 Returns a normalized version of the input vector.
 \param a The vector to be normalized.
 \return The normalized vector.
 */
vector4 normalize(vector4 a);

#endif
