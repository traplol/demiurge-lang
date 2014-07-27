#ifndef _DEMIURGE_JIT_OUTPUT_FUNCTION_H
#define _DEMIURGE_JIT_OUTPUT_FUNCTIONS_H
#include <stdio.h>
static void print(const char* string) {
    printf(string);
}

static void println(const char* string) {
    printf("%s\n", string);
}

static void printd(double x) {
    printf("%f\n", x);
}

static void printi(unsigned long long int x) {
    printf("%llu\n", x);
}

static void printc(unsigned long long int x) {
    putchar((char)x);
}


#endif