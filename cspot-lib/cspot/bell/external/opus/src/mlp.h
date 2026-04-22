

#ifndef _MLP_H_
#define _MLP_H_

#include "opus_types.h"

#define WEIGHTS_SCALE (1.f/128)

#define MAX_NEURONS 32

typedef struct {
  const opus_int8 *bias;
  const opus_int8 *input_weights;
  int nb_inputs;
  int nb_neurons;
  int sigmoid;
} DenseLayer;

typedef struct {
  const opus_int8 *bias;
  const opus_int8 *input_weights;
  const opus_int8 *recurrent_weights;
  int nb_inputs;
  int nb_neurons;
} GRULayer;

extern const DenseLayer layer0;
extern const GRULayer layer1;
extern const DenseLayer layer2;

void compute_dense(const DenseLayer *layer, float *output, const float *input);

void compute_gru(const GRULayer *gru, float *state, const float *input);

#endif
