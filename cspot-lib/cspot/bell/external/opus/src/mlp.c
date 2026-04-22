

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include "opus_types.h"
#include "opus_defines.h"
#include "arch.h"
#include "tansig_table.h"
#include "mlp.h"

static OPUS_INLINE float tansig_approx(float x)
{
    int i;
    float y, dy;
    float sign=1;

    if (!(x<8))
        return 1;
    if (!(x>-8))
        return -1;
#ifndef FIXED_POINT

    if (celt_isnan(x))
       return 0;
#endif
    if (x<0)
    {
       x=-x;
       sign=-1;
    }
    i = (int)floor(.5f+25*x);
    x -= .04f*i;
    y = tansig_table[i];
    dy = 1-y*y;
    y = y + x*dy*(1 - y*x);
    return sign*y;
}

static OPUS_INLINE float sigmoid_approx(float x)
{
   return .5f + .5f*tansig_approx(.5f*x);
}

static void gemm_accum(float *out, const opus_int8 *weights, int rows, int cols, int col_stride, const float *x)
{
   int i, j;
   for (i=0;i<rows;i++)
   {
      for (j=0;j<cols;j++)
         out[i] += weights[j*col_stride + i]*x[j];
   }
}

void compute_dense(const DenseLayer *layer, float *output, const float *input)
{
   int i;
   int N, M;
   int stride;
   M = layer->nb_inputs;
   N = layer->nb_neurons;
   stride = N;
   for (i=0;i<N;i++)
      output[i] = layer->bias[i];
   gemm_accum(output, layer->input_weights, N, M, stride, input);
   for (i=0;i<N;i++)
      output[i] *= WEIGHTS_SCALE;
   if (layer->sigmoid) {
      for (i=0;i<N;i++)
         output[i] = sigmoid_approx(output[i]);
   } else {
      for (i=0;i<N;i++)
         output[i] = tansig_approx(output[i]);
   }
}

void compute_gru(const GRULayer *gru, float *state, const float *input)
{
   int i;
   int N, M;
   int stride;
   float tmp[MAX_NEURONS];
   float z[MAX_NEURONS];
   float r[MAX_NEURONS];
   float h[MAX_NEURONS];
   M = gru->nb_inputs;
   N = gru->nb_neurons;
   stride = 3*N;

   for (i=0;i<N;i++)
      z[i] = gru->bias[i];
   gemm_accum(z, gru->input_weights, N, M, stride, input);
   gemm_accum(z, gru->recurrent_weights, N, N, stride, state);
   for (i=0;i<N;i++)
      z[i] = sigmoid_approx(WEIGHTS_SCALE*z[i]);

   for (i=0;i<N;i++)
      r[i] = gru->bias[N + i];
   gemm_accum(r, &gru->input_weights[N], N, M, stride, input);
   gemm_accum(r, &gru->recurrent_weights[N], N, N, stride, state);
   for (i=0;i<N;i++)
      r[i] = sigmoid_approx(WEIGHTS_SCALE*r[i]);

   for (i=0;i<N;i++)
      h[i] = gru->bias[2*N + i];
   for (i=0;i<N;i++)
      tmp[i] = state[i] * r[i];
   gemm_accum(h, &gru->input_weights[2*N], N, M, stride, input);
   gemm_accum(h, &gru->recurrent_weights[2*N], N, N, stride, tmp);
   for (i=0;i<N;i++)
      h[i] = z[i]*state[i] + (1-z[i])*tansig_approx(WEIGHTS_SCALE*h[i]);
   for (i=0;i<N;i++)
      state[i] = h[i];
}

