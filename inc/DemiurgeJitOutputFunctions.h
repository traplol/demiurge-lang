#ifndef _DEMIURGE_JIT_OUTPUT_FUNCTIONS_H
#define _DEMIURGE_JIT_OUTPUT_FUNCTIONS_H
#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
#define EXPORT __declspec( dllexport )
#else
#define EXPORT
#endif

typedef unsigned long long int uint64;

extern "C" {

    EXPORT int print(const char *string) {
        printf(string);
        return strlen(string);
    }

    EXPORT int println(const char *string) {
        printf("%s\n", string);
        return strlen(string);
    }

    EXPORT double printd(double x) {
        printf("%f", x);
        return x;
    }

    EXPORT uint64 printi(uint64 x) {
        printf("%llu", x);
        return x;
    }

    EXPORT uint64 printc(uint64 x) {
        putchar((char)x);
        return x;
    }

}

#undef EXPORT
#endif
