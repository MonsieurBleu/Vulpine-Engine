#ifndef FNCT_FAST_MATHS_GLSL
#define FNCT_FAST_MATHS_GLSL

float fastinverseSqrt(float x) {
    float xhalf = 0.5 * x;
    int i = floatBitsToInt(x);
    i = 0x5f3759df - (i >> 1);
    x = intBitsToFloat(i);
    x = x * (1.5 - xhalf * x * x);
    return x;
}

#endif