

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "stack_alloc.h"
#include "kiss_fft.h"
#include "mathops.h"
#include "modes.h"

#ifndef M_PI
#define M_PI 3.141592653
#endif

int ret = 0;

void check(kiss_fft_cpx  * in,kiss_fft_cpx  * out,int nfft,int isinverse)
{
    int bin,k;
    double errpow=0,sigpow=0, snr;

    for (bin=0;bin<nfft;++bin) {
        double ansr = 0;
        double ansi = 0;
        double difr;
        double difi;

        for (k=0;k<nfft;++k) {
            double phase = -2*M_PI*bin*k/nfft;
            double re = cos(phase);
            double im = sin(phase);
            if (isinverse)
                im = -im;

            if (!isinverse)
            {
               re /= nfft;
               im /= nfft;
            }

            ansr += in[k].r * re - in[k].i * im;
            ansi += in[k].r * im + in[k].i * re;
        }

        difr = ansr - out[bin].r;
        difi = ansi - out[bin].i;
        errpow += difr*difr + difi*difi;
        sigpow += ansr*ansr+ansi*ansi;
    }
    snr = 10*log10(sigpow/errpow);
    printf("nfft=%d inverse=%d,snr = %f\n",nfft,isinverse,snr );
    if (snr<60) {
       printf( "** poor snr: %f ** \n", snr);
       ret = 1;
    }
}

void test1d(int nfft,int isinverse,int arch)
{
    size_t buflen = sizeof(kiss_fft_cpx)*nfft;
    kiss_fft_cpx *in;
    kiss_fft_cpx *out;
    int k;
#ifdef CUSTOM_MODES
    kiss_fft_state *cfg = opus_fft_alloc(nfft,0,0,arch);
#else
    int id;
    const kiss_fft_state *cfg;
    CELTMode *mode = opus_custom_mode_create(48000, 960, NULL);
    if (nfft == 480) id = 0;
    else if (nfft == 240) id = 1;
    else if (nfft == 120) id = 2;
    else if (nfft == 60) id = 3;
    else return;
    cfg = mode->mdct.kfft[id];
#endif

    in = (kiss_fft_cpx*)malloc(buflen);
    out = (kiss_fft_cpx*)malloc(buflen);

    for (k=0;k<nfft;++k) {
        in[k].r = (rand() % 32767) - 16384;
        in[k].i = (rand() % 32767) - 16384;
    }

    for (k=0;k<nfft;++k) {
       in[k].r *= 32768;
       in[k].i *= 32768;
    }

    if (isinverse)
    {
       for (k=0;k<nfft;++k) {
          in[k].r /= nfft;
          in[k].i /= nfft;
       }
    }

    if (isinverse)
       opus_ifft(cfg,in,out, arch);
    else
       opus_fft(cfg,in,out, arch);

    check(in,out,nfft,isinverse);

    free(in);
    free(out);
#ifdef CUSTOM_MODES
    opus_fft_free(cfg, arch);
#endif
}

int main(int argc,char ** argv)
{
    ALLOC_STACK;
    int arch = opus_select_arch();

    if (argc>1) {
        int k;
        for (k=1;k<argc;++k) {
            test1d(atoi(argv[k]),0,arch);
            test1d(atoi(argv[k]),1,arch);
        }
    }else{
        test1d(32,0,arch);
        test1d(32,1,arch);
        test1d(128,0,arch);
        test1d(128,1,arch);
        test1d(256,0,arch);
        test1d(256,1,arch);
#ifndef RADIX_TWO_ONLY
        test1d(36,0,arch);
        test1d(36,1,arch);
        test1d(50,0,arch);
        test1d(50,1,arch);
        test1d(60,0,arch);
        test1d(60,1,arch);
        test1d(120,0,arch);
        test1d(120,1,arch);
        test1d(240,0,arch);
        test1d(240,1,arch);
        test1d(480,0,arch);
        test1d(480,1,arch);
#endif
    }
    return ret;
}
