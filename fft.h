#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// 定义复数结构
typedef struct {
    double real;
    double imag;
} Complex;

void fft(Complex* x, int N);
double magnitude(Complex c);
void removeDCOffset(Complex signal[], int n);