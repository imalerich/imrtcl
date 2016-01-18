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

struct vector4 vector4();
struct vector4 vector4_init(float x, float y, float z, float w);

#endif
