#ifndef CUSTOM_TYPES_HP
#define CUSTOM_TYPES_H

#include <stdint.h>
#include <float.h>
#include <inttypes.h>

#define global_variable static
#define local_persist static
#define internal static

typedef unsigned char uchar;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
#define S8_MIN INT8_MIN
#define S8_MAX INT8_MAX 
#define S16_MIN INT16_MIN 
#define S16_MAX INT16_MAX 
#define S32_MIN INT32_MIN 
#define S32_MAX INT32_MAX
#define S64_MIN INT64_MIN 
#define S64_MAX INT64_MAX 

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#define U8_MIN UINT8_MIN
#define U8_MAX UINT8_MAX 
#define U16_MIN UINT16_MIN 
#define U16_MAX UINT16_MAX 
#define U32_MIN UINT32_MIN 
#define U32_MAX UINT32_MAX
#define U64_MIN UINT64_MIN 
#define U64_MAX UINT64_MAX 

typedef float f32;
typedef double f64;
#define F32_MIN -FLT_MIN
#define F32_MAX FLT_MAX
#define F64_MIN -DBL_MIN
#define F64_MAX DBL_MAX

typedef intptr_t sptr;
typedef uintptr_t uptr;

#endif //CUSTOM_TYPES_H