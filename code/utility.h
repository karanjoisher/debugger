#ifndef UTILITY_H
#define UTILIY_H

#include <stdio.h>

#ifdef DEBUG_BUILD
#define ASSERT(expression) if(!(expression)) {*(volatile int *)0 = 0;}
#else
#define ASSERT(expression)
#endif

#define INVALID_CODE_PATH  ASSERT(!"InvalidCodePath")

#define OUT(...) fprintf(stdout, __VA_ARGS__)
#define ERR(...) fprintf(stderr, __VA_ARGS__)
#define ERR_WITH_LOCATION_INFO(...) fprintf(stderr, __VA_ARGS__); fprintf(stderr, " @ %s:%s(%d)\n", __func__, __FILE__, __LINE__) 

#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

#define KILOBYTES(value) ((value)*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)
#define GIGABYTES(value) (MEGABYTES(value)*1024LL)
#define TERABYTES(value) (GIGABYTES(value)*1024LL)

#define LOW_32_FROM_64(value) ((value) & 0x00000000FFFFFFFF)
#endif //UTILIY_H