//
//  vector.h
//  OpenCL-Mac
//
//  Created by Ian Malerich on 1/18/16.
//  Copyright © 2016 Ian Malerich. All rights reserved.
//

#ifndef vector_h
#define vector_h

struct vector4 {
    float x;
    float y;
    float z;
    float w;
};

/**
 Initialize a vector4 to { 0.0f, 0.0f, 0.0f, 0.0f }.
 */
struct vector4 vector4();

/**
 Initializes a vector4 to { 1.0f, 0.0f, 0.0f, 0.0f }.
 This quaternion represents no rotation.
 */
struct vector4 quaternion();

/**
Initialize a vector4 to { x, y, z, 0.0f }.
 */
struct vector4 vector3_init(float x, float y, float z);

/**
 Initialize a vector4 to { x, y, z, w }.
 */
struct vector4 vector4_init(float x, float y, float z, float w);

/**
 Constructs a quaternion vector from the input 'axis' (the w
 component of that vector will be ignored), and the input theta.
 \param axis Axis of rotation.
 \param theta Angle of rotation in radians.
 */
struct vector4 vector4_quaternion(struct vector4 axis, float theta);

/**
 Multiplies the two input vectors per component, the result is then
 returned.
 \param a First operand of multiplication.
 \param b Second operand of multiplication.
 \return The result of the multiplication.
 */
struct vector4 vector4_mult(struct vector4 a, struct vector4 b);

/**
 Multiplies two vectors representing quaternions, the resulting
 vector will represent the rotations of both quaternions.
 \param a Firt operand of multiplication.
 \param b Second operand of multiplication.
 \return A quaternion representing the rotations in 'a' and 'b'.
 */
struct vector4 quaternion_mult(struct vector4 a, struct vector4 b);

/**
 Vector cross product. For simplicity this method will ignore
 the 'w' components of each input vector, and treats the
 vectors as 3 dimmensional.
 \param a First operand of the dot product.
 \param b Second operand of the dot product.
 \return Vector result of the cross product.
 */
struct vector4 cross3(struct vector4 a, struct vector4 b);

/**
 Vector dot product.
 \param a First operand of the dot product.
 \param b Second operand of the dot product.
 \return Scalar result of the dot product.
 */
float dot(struct vector4 a, struct vector4 b);

/**
 Determines the length of the input vector.
 \param a The vector to find the length of.
 \return The length of the input vector.
 */
float length(struct vector4 a);

/**
 Returns a normalized version of the input vector.
 \param a The vector to be normalized.
 \return The normalized vector.
 */
struct vector4 normalize(struct vector4 a);

#endif
