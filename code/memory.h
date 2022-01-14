/* date = December 17th 2021 6:33 pm */

#ifndef MEMORY_H
#define MEMORY_H

typedef struct memory_pool_t
{
    size_t platSize;
    u8 *platBase;
    size_t permSize;
    u8 *permBase;
    size_t tempSize;
    u8 *tempBase;
} memory_pool_t;

typedef struct memory_arena_t
{
    u8 *base;
    size_t used;
    size_t size;
    u32 tempCount;
} memory_arena_t;

typedef struct temp_memory_t
{
    memory_arena_t *arena;
    size_t usedOld;
} temp_memory_t;

typedef struct string_array_t
{
    u32 count;
    char **data;
} string_array_t;

internal temp_memory_t
begin_temp_memory(memory_arena_t *arena)
{
    temp_memory_t result = 
    {
        .arena = arena,
        .usedOld = arena->used,
    };
    
    arena->tempCount += 1;
    
    return result;
}

internal void
end_temp_memory(temp_memory_t mem)
{
    memory_arena_t *arena = mem.arena;
    assert(arena->used >= mem.usedOld);
    arena->used = mem.usedOld;
    arena->tempCount -= 1;
}

internal void
check_arena(memory_arena_t *arena)
{
    assert(arena->tempCount == 0);
}

inline void
memory_pool_init(memory_pool_t *pool, u8 *platAddress, size_t platSize,
                 u8 *address, size_t permSize, size_t tempSize)
{
    pool->platBase = platAddress;
    pool->platSize = platSize;
    pool->permBase = address;
    pool->permSize = permSize;
    pool->tempBase = address + permSize;
    pool->tempSize = tempSize;
}

inline void
arena_init(memory_arena_t *arena, size_t size, u8 *base)
{
    arena->size = size;
    arena->used = 0;
    arena->base = base;
}

inline size_t
arena_get_alignment_offset(memory_arena_t *arena, size_t alignment)
{
    size_t alignmentOffset = 0;
    
    size_t resultPointer = (size_t)arena->base + arena->used;
    size_t alignmentMask = alignment - 1;
    if(resultPointer & alignmentMask)
    {
        alignmentOffset = alignment - (resultPointer & alignmentMask);
    }
    
    return(alignmentOffset);
}

#define push_struct(arena, type, ...) (type *)push_size_(arena, sizeof(type), ## __VA_ARGS__)
#define push_array(arena, count, type, ...) (type *)push_size_(arena, (count)*sizeof(type), ## __VA_ARGS__)
#define push_size(arena, size, ...) push_size_(arena, size, ## __VA_ARGS__)

inline size_t
arena_get_effective_size(memory_arena_t *arena, size_t sizeInit, size_t alignment)
{
    size_t size = sizeInit;
    
    size_t alignmentOffset = arena_get_alignment_offset(arena, alignment);
    size += alignmentOffset;
    
    return(size);
}

inline void *
push_size_(memory_arena_t *arena, size_t sizeInit, size_t alignment)
{
    void *result = 0;
    
    size_t size = arena_get_effective_size(arena, sizeInit, alignment);
    
    assert((arena->used + size) <= arena->size);
    
    size_t alignmentOffset = arena_get_alignment_offset(arena, alignment);
    uintptr_t offsetInBlock = arena->used + alignmentOffset;
    result = arena->base + offsetInBlock;
    arena->used += size;
    
    assert(size >= sizeInit);
    
    // NOTE(casey): This is just to guarantee that nobody passed in an alignment
    // on their first allocation that was _greater_ that than the page alignment
    assert(arena->used <= arena->size);
    
    return(result);
}

internal string_array_t
make_string_array(u32 count)
{
    string_array_t result;
    
    result.count = count;
    result.data = (char **)malloc(count * sizeof(char *));
    
    return result;
}

internal u32
string_length(char *string)
{
    u32 length = 0;
    
    char *at = string;
    while (*at)
    {
        at++;
        length++;
    }
    
    return length;
}

internal char *
string_allocate(char *string)
{
    u32 length = string_length(string);
    char *str = (char *)malloc((length + 1) * sizeof(char));
    
    memcpy(str, string, length);
    str[length] = 0;
    
    return str;
}

internal b32
string_equal(char *a, char *b)
{
    if (strcmp(a, b) == 0)
    {
        return 1;
    }
    
    return 0;
}

internal b32
strings_are_equal(char* src, char* dst, size_t dstlen)
{
    while (*src && dstlen-- && *dst)
    {
        if (*src++ != *dst++)
        {
            return 0;
        }
    }
    
    return (dstlen && *src == *dst) || (!dstlen && *src == 0);
}

internal void
string_concat(char *dest, size_t destSize, char *a, char *b)
{
    u32 aLength = string_length(a);
    u32 bLength = string_length(b);
    
    assert((aLength + bLength) <= destSize);
    
    memcpy(dest, a, aLength);
    memcpy(dest + aLength, b, bLength);
    dest[aLength + bLength] = 0;
}

#endif //MEMORY_H
