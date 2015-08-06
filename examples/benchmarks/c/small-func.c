#include <time.h>
#include <stdio.h>


int smallFunc(int x) {
    return x;
}

int main(){
    clock_t start = clock();
    int iters = 500000000;
    int last;
    for (int x = 1; x <= iters; x = x + 1) {
        last = smallFunc(x);
    }
    clock_t stop = clock();
    printf("%d calls in %ldms\n",last, stop-start);
    return 0;
}
