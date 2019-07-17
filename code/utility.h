#ifndef UTILITY_H
#define UTILIY_H

#include <stdio.h>

#ifdef DEBUG_BUILD
#define ASSERT(expression) if(!(expression)) {*(volatile int *)0 = 0;}
#else
#define ASSERT(expression)
#endif

#define INVALID_CODE_PATH  ASSERT(!"InvalidCodePath")
#define INVALID_DEFAULT_CASE  default:ASSERT(!"InvalidCodePath")

#define OUT(...) fprintf(stdout, __VA_ARGS__)
#define ERR(...) fprintf(stderr, __VA_ARGS__)
#define ERR_WITH_LOCATION_INFO(...) fprintf(stderr, __VA_ARGS__); fprintf(stderr, " @ %s:%s(%d)\n", __func__, __FILE__, __LINE__) 

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

#define KILOBYTES(value) ((value)*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)
#define GIGABYTES(value) (MEGABYTES(value)*1024LL)
#define TERABYTES(value) (GIGABYTES(value)*1024LL)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ALIGN4(a) (((a) + 3) & ~3)
#define ALIGN_POW_2(a, p) (((a) + (p-1)) & ~(p-1))
#define ALIGN8(a) (((a) + 7) & ~7)
#define ALIGN16(a) (((a) + 15) & ~15)
#define ALIGN32(a) (((a) + 31) & ~31)
#define ALIGN64(a) (((a) + 63) & ~63)

inline void ArrayCopy(void *source, void *destination, u32 size)
{
    char *s = (char*)source;
    char *d = (char*)destination;
    while(size--)
    {
        *d++ = *s++;
    }
}


inline void ClearArray(void *array, u32 size, u8 val = 0)
{
    u8 *dest = (u8*)array;
    while(size--)
    {
        *dest++ = val;
    }
}


#define COPY_DW_TO_LOWDW_OF_QW(s, d) ArrayCopy(&(s), &(d) , sizeof(u32)) 
#define COPY_DW_TO_HIGHDW_OF_QW(s, d) ArrayCopy(&(s), (((u8*)(&(d))) + sizeof(u32)) , sizeof(u32)) 

#define COPY_LOWDW_OF_QW_TO_DW(s, d) ArrayCopy(&(s), &(d) , sizeof(u32)) 
#define COPY_HIGHDW_OF_QW_TO_DW(s, d) ArrayCopy((((u8*)(&(s))) + sizeof(u32)), &(d) , sizeof(u32)) 

#endif //UTILIY_H