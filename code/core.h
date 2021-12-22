/* date = December 17th 2021 10:27 pm */

#ifndef CORE_H
#define CORE_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define internal static
#define global static

#define array_count(a) (sizeof(a) / sizeof((a)[0]))

typedef int32_t b32;

typedef int8_t s8;
typedef uint8_t u8;

typedef int8_t s08;
typedef uint8_t u08;

typedef int16_t s16;
typedef uint16_t u16;

typedef int32_t s32;
typedef uint32_t u32;

typedef int64_t s64;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef float r32;
typedef double r64;

typedef struct indexed_b32
{
    b32 found;
    u32 index;
} indexed_b32;

typedef struct typeless_vector
{
    u32 count;
    void *data;
} typeless_vector;

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

internal u32
clamp(u32 value, u32 min, u32 max)
{
    u32 result = value;
    
    if (result < min)
    {
        result = min;
    }
    
    if (result > max)
    {
        result = max;
    }
    
    return result;
}

#include "memory.h"
#include "fileio.h"

#endif //CORE_H
