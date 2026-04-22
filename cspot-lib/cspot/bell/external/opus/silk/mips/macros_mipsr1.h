

#ifndef __SILK_MACROS_MIPSR1_H__
#define __SILK_MACROS_MIPSR1_H__

#define mips_clz(x) __builtin_clz(x)

#undef silk_SMULWB
static inline int silk_SMULWB(int a, int b)
{
    long long ac;
    int c;

    ac = __builtin_mips_mult(a, (opus_int32)(opus_int16)b);
    c = __builtin_mips_extr_w(ac, 16);

    return c;
}

#undef silk_SMLAWB
#define silk_SMLAWB(a32, b32, c32)       ((a32) + silk_SMULWB(b32, c32))

#undef silk_SMULWW
static inline int silk_SMULWW(int a, int b)
{
    long long ac;
    int c;

    ac = __builtin_mips_mult(a, b);
    c = __builtin_mips_extr_w(ac, 16);

    return c;
}

#undef silk_SMLAWW
static inline int silk_SMLAWW(int a, int b, int c)
{
    long long ac;
    int res;

    ac = __builtin_mips_mult(b, c);
    res = __builtin_mips_extr_w(ac, 16);
    res += a;

    return res;
}

#define OVERRIDE_silk_CLZ16
static inline opus_int32 silk_CLZ16(opus_int16 in16)
{
    int re32;
    opus_int32 in32 = (opus_int32 )in16;
    re32 = mips_clz(in32);
    re32-=16;
    return re32;
}

#define OVERRIDE_silk_CLZ32
static inline opus_int32 silk_CLZ32(opus_int32 in32)
{
    int re32;
    re32 = mips_clz(in32);
    return re32;
}

#endif
