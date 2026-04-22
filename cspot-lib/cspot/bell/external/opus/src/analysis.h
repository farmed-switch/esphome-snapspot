

#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "celt.h"
#include "opus_private.h"
#include "mlp.h"

#define NB_FRAMES 8
#define NB_TBANDS 18
#define ANALYSIS_BUF_SIZE 720

#define ANALYSIS_COUNT_MAX 10000

#define DETECT_SIZE 100

typedef struct {
   int arch;
   int application;
   opus_int32 Fs;
#define TONALITY_ANALYSIS_RESET_START angle
   float angle[240];
   float d_angle[240];
   float d2_angle[240];
   opus_val32 inmem[ANALYSIS_BUF_SIZE];
   int   mem_fill;
   float prev_band_tonality[NB_TBANDS];
   float prev_tonality;
   int prev_bandwidth;
   float E[NB_FRAMES][NB_TBANDS];
   float logE[NB_FRAMES][NB_TBANDS];
   float lowE[NB_TBANDS];
   float highE[NB_TBANDS];
   float meanE[NB_TBANDS+1];
   float mem[32];
   float cmean[8];
   float std[9];
   float Etracker;
   float lowECount;
   int E_count;
   int count;
   int analysis_offset;
   int write_pos;
   int read_pos;
   int read_subframe;
   float hp_ener_accum;
   int initialized;
   float rnn_state[MAX_NEURONS];
   opus_val32 downmix_state[3];
   AnalysisInfo info[DETECT_SIZE];
} TonalityAnalysisState;

void tonality_analysis_init(TonalityAnalysisState *analysis, opus_int32 Fs);

void tonality_analysis_reset(TonalityAnalysisState *analysis);

void tonality_get_info(TonalityAnalysisState *tonal, AnalysisInfo *info_out, int len);

void run_analysis(TonalityAnalysisState *analysis, const CELTMode *celt_mode, const void *analysis_pcm,
                 int analysis_frame_size, int frame_size, int c1, int c2, int C, opus_int32 Fs,
                 int lsb_depth, downmix_func downmix, AnalysisInfo *analysis_info);

#endif
