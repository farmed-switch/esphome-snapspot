

#ifndef CELT_FIXED_GENERIC_MIPSR1_H
#define CELT_FIXED_GENERIC_MIPSR1_H

#undef MULT16_32_Q15_ADD
static inline int MULT16_32_Q15_ADD(int a, int b, int c, int d) {
    int m;
    asm volatile("MULT $ac1, %0, %1" : : "r" ((int)a), "r" ((int)b));
    asm volatile("madd $ac1, %0, %1" : : "r" ((int)c), "r" ((int)d));
    asm volatile("EXTR.W %0,$ac1, %1" : "=r" (m): "i" (15));
    return m;
}

#undef MULT16_32_Q15_SUB
static inline int MULT16_32_Q15_SUB(int a, int b, int c, int d) {
    int m;
    asm volatile("MULT $ac1, %0, %1" : : "r" ((int)a), "r" ((int)b));
    asm volatile("msub $ac1, %0, %1" : : "r" ((int)c), "r" ((int)d));
    asm volatile("EXTR.W %0,$ac1, %1" : "=r" (m): "i" (15));
    return m;
}

#undef MULT16_16_Q15_ADD
static inline int MULT16_16_Q15_ADD(int a, int b, int c, int d) {
    int m;
    asm volatile("MULT $ac1, %0, %1" : : "r" ((int)a), "r" ((int)b));
    asm volatile("madd $ac1, %0, %1" : : "r" ((int)c), "r" ((int)d));
    asm volatile("EXTR.W %0,$ac1, %1" : "=r" (m): "i" (15));
    return m;
}

#undef MULT16_16_Q15_SUB
static inline int MULT16_16_Q15_SUB(int a, int b, int c, int d) {
    int m;
    asm volatile("MULT $ac1, %0, %1" : : "r" ((int)a), "r" ((int)b));
    asm volatile("msub $ac1, %0, %1" : : "r" ((int)c), "r" ((int)d));
    asm volatile("EXTR.W %0,$ac1, %1" : "=r" (m): "i" (15));
    return m;
}

#undef MULT16_32_Q16
static inline int MULT16_32_Q16(int a, int b)
{
    int c;
    asm volatile("MULT $ac1,%0, %1" : : "r" (a), "r" (b));
    asm volatile("EXTR.W %0,$ac1, %1" : "=r" (c): "i" (16));
    return c;
}

#undef MULT16_32_P16
static inline int MULT16_32_P16(int a, int b)
{
    int c;
    asm volatile("MULT $ac1, %0, %1" : : "r" (a), "r" (b));
    asm volatile("EXTR_R.W %0,$ac1, %1" : "=r" (c): "i" (16));
    return c;
}

#undef MULT16_32_Q15
static inline int MULT16_32_Q15(int a, int b)
{
    int c;
    asm volatile("MULT $ac1, %0, %1" : : "r" (a), "r" (b));
    asm volatile("EXTR.W %0,$ac1, %1" : "=r" (c): "i" (15));
    return c;
}

#undef MULT32_32_Q31
static inline int MULT32_32_Q31(int a, int b)
{
    int r;
    asm volatile("MULT $ac1, %0, %1" : : "r" (a), "r" (b));
    asm volatile("EXTR.W %0,$ac1, %1" : "=r" (r): "i" (31));
    return r;
}

#undef PSHR32
static inline int PSHR32(int a, int shift)
{
    int r;
    asm volatile ("SHRAV_R.W %0, %1, %2" :"=r" (r): "r" (a), "r" (shift));
    return r;
}

#undef MULT16_16_P15
static inline int MULT16_16_P15(int a, int b)
{
    int r;
    asm volatile ("mul %0, %1, %2" :"=r" (r): "r" (a), "r" (b));
    asm volatile ("SHRA_R.W %0, %1, %2" : "+r" (r):  "0" (r), "i"(15));
    return r;
}

#endif
