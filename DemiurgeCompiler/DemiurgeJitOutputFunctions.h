#ifndef _DEMIURGE_JIT_OUTPUT_FUNCTION_H
#define _DEMIURGE_JIT_OUTPUT_FUNCTIONS_H
#include <stdio.h>
#include <stdarg.h>
static void print(const char* string) {
    printf(string);
}

static void println(const char* string) {
    printf("%s\n", string);
}

static void _printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

static void printd(double x) {
    printf("%f", x);
}

static void printi(unsigned long long int x) {
    printf("%llu", x);
}

static void printc(unsigned long long int x) {
    putchar((char)x);
}


#endif