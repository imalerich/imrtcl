#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "surface.h"

cl_float * importModel(const char * filename, size_t * triangle_count);

#endif
