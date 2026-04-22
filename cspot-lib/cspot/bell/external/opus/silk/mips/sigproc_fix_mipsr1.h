

#ifndef SILK_SIGPROC_FIX_MIPSR1_H
#define SILK_SIGPROC_FIX_MIPSR1_H

#undef silk_SAT16
static inline short int silk_SAT16(int a)
{
    int c;
    c = __builtin_mips_shll_s_w(a, 16);
    c = c>>16;

    return c;
}

#undef silk_LSHIFT_SAT32
static inline int silk_LSHIFT_SAT32(int a, int shift)
{
    int r;

    r = __builtin_mips_shll_s_w(a, shift);

    return r;
}

#undef silk_RSHIFT_ROUND
static inline int silk_RSHIFT_ROUND(int a, int shift)
{
    int r;

    r = __builtin_mips_shra_r_w(a, shift);
    return r;
}

#endif
