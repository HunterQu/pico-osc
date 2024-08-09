#include "fft.h"

double magnitude(Complex c) {
    return sqrt(c.real * c.real + c.imag * c.imag);
}
// 去直流分量
void removeDCOffset(Complex signal[], int n) {
    double mean = 0.0;
    for (int i = 0; i < n; ++i) {
        mean += signal[i].real;
    }
    mean /= n;
    for (int i = 0; i < n; ++i) {
        signal[i].real -= mean;
    }
}
// 实现快速傅里叶变换（FFT）
void fft(Complex *x, int n) {
    if (n <= 1) return;

    // 将输入数据分成奇数和偶数部分
    Complex even[n / 2];
    Complex odd[n / 2];
    for (int i = 0; i < n / 2; i++) {
        even[i] = x[i * 2];
        odd[i] = x[i * 2 + 1];
    }

    // 递归计算FFT
    fft(even, n / 2);
    fft(odd, n / 2);

    // 合并结果
    for (int k = 0; k < n / 2; k++) {
        double t = -2 * M_PI * k / n;
        Complex tComplex = {cos(t) * odd[k].real - sin(t) * odd[k].imag,
                            sin(t) * odd[k].real + cos(t) * odd[k].imag};
        x[k].real = even[k].real + tComplex.real;
        x[k].imag = even[k].imag + tComplex.imag;
        x[k + n / 2].real = even[k].real - tComplex.real;
        x[k + n / 2].imag = even[k].imag - tComplex.imag;
    }
}