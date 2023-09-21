#ifndef UTILS_H_
#define UTILS_H_

#define LENGTH_OF(x) (sizeof(x)/sizeof(*x))


#ifdef _DEBUG
    #include <stdio.h>
    #include <stdlib.h>

    #define UNREACHABLE()                                                           \
    do {                                                                            \
        fprintf(stderr, "%s:%d: Unreachable line reached.\n", __FILE__, __LINE__);  \
        exit(666);                                                                  \
    } while (0)
#else
    #ifdef _WIN32
        #define UNREACHABLE()   __assume(0)
    #else
        #define UNREACHABLE()   __builtin_unreachable()
    #endif
#endif


#ifdef _DEBUG
    #define ASSERT(...) \
    do { \
        if (!(__VA_ARGS__)) { \
            fprintf(stderr, "%s:%d: Assert failed! (%s)\n", __FILE__, __LINE__, #__VA_ARGS__); \
            exit(1); \
        } \
    } while(0)
#else
    #define ASSERT(...)
#endif


#include <stdint.h>

typedef double   f64;
typedef float    f32;
typedef uint64_t u64;
typedef int64_t  i64;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint16_t u16;
typedef int16_t  i16;
typedef uint8_t  u8;
typedef int8_t   i8;
typedef uint32_t b32; // semantic only



#endif
