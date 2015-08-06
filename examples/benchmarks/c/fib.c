#include <time.h>
#include <stdio.h>

int fib(int n) {
    if (n < 2) {
        return n;
    }
    
    int prevPrev = 0;
    int prev = 1;
    int result = 0;
    
    for (int i = 0; i <= n; i = i + 1) {
        result = prev + prevPrev;
        prevPrev = prev;
        prev = result;
    }
    
    return result;
}

int main() {
    clock_t start = clock();
    int f = fib(500000000);
    clock_t stop = clock();
    printf("%d in %ldms\n", f, stop-start);
    return 0;
}
