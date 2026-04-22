

#ifndef MAPPING_MATRIX_H
#define MAPPING_MATRIX_H

#include "opus_types.h"
#include "opus_projection.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MappingMatrix
{
    int rows;
    int cols;
    int gain;

} MappingMatrix;

opus_int32 mapping_matrix_get_size(int rows, int cols);

opus_int16 *mapping_matrix_get_data(const MappingMatrix *matrix);

void mapping_matrix_init(
    MappingMatrix * const matrix,
    int rows,
    int cols,
    int gain,
    const opus_int16 *data,
    opus_int32 data_size
);

#ifndef DISABLE_FLOAT_API
void mapping_matrix_multiply_channel_in_float(
    const MappingMatrix *matrix,
    const float *input,
    int input_rows,
    opus_val16 *output,
    int output_row,
    int output_rows,
    int frame_size
);

void mapping_matrix_multiply_channel_out_float(
    const MappingMatrix *matrix,
    const opus_val16 *input,
    int input_row,
    int input_rows,
    float *output,
    int output_rows,
    int frame_size
);
#endif

void mapping_matrix_multiply_channel_in_short(
    const MappingMatrix *matrix,
    const opus_int16 *input,
    int input_rows,
    opus_val16 *output,
    int output_row,
    int output_rows,
    int frame_size
);

void mapping_matrix_multiply_channel_out_short(
    const MappingMatrix *matrix,
    const opus_val16 *input,
    int input_row,
    int input_rows,
    opus_int16 *output,
    int output_rows,
    int frame_size
);

extern const MappingMatrix mapping_matrix_foa_mixing;
extern const opus_int16 mapping_matrix_foa_mixing_data[36];

extern const MappingMatrix mapping_matrix_soa_mixing;
extern const opus_int16 mapping_matrix_soa_mixing_data[121];

extern const MappingMatrix mapping_matrix_toa_mixing;
extern const opus_int16 mapping_matrix_toa_mixing_data[324];

extern const MappingMatrix mapping_matrix_foa_demixing;
extern const opus_int16 mapping_matrix_foa_demixing_data[36];

extern const MappingMatrix mapping_matrix_soa_demixing;
extern const opus_int16 mapping_matrix_soa_demixing_data[121];

extern const MappingMatrix mapping_matrix_toa_demixing;
extern const opus_int16 mapping_matrix_toa_demixing_data[324];

#ifdef __cplusplus
}
#endif

#endif
