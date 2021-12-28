/* date = December 17th 2021 10:27 pm */

#ifndef CORE_H
#define CORE_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "types.h"

#define array_count(a) (sizeof(a) / sizeof((a)[0]))

internal typeless_vector
make_typeless_vector(u32 count, u32 dataSize)
{
    typeless_vector result =
    {
        .count = count,
        .data = malloc(count * dataSize),
    };
    
    return result;
}

#include "memory.h"
#include "fileio.h"

#include "math/math_core.h"

#endif //CORE_H
