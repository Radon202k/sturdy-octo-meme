/* date = December 24th 2021 7:27 am */

#ifndef TYPES_H
#define TYPES_H

#define internal static
#define global static

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

#define kilobytes(value) ((value) * 1024)
#define megabytes(value) (kilobytes(value) * 1024)
#define gigabytes(value) (megabytes(value) * 1024LL)
#define terabytes(value) (gigabytes(value) * 1024LL)

#endif //TYPES_H
