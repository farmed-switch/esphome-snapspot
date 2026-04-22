

#ifndef TNS_DECODE_COEF_H
#define TNS_DECODE_COEF_H

#ifdef __cplusplus
extern "C"
{
#endif

    Int tns_decode_coef(
        const Int   order,
        const Int   coef_res,
        Int32 lpc_coef[],
        Int32 scratchTnsDecCoefMem[]);

#ifdef __cplusplus
}
#endif

#endif
