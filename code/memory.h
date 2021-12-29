/* date = December 17th 2021 6:33 pm */

#ifndef MEMORY_H
#define MEMORY_H

typedef struct memory_arena
{
    int placeholder;
} memory_arena;

typedef struct string_array
{
    u32 count;
    char **data;
} string_array;

internal string_array
make_string_array(u32 count)
{
    string_array result;
    
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

#endif //MEMORY_H
