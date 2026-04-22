

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "opus.h"
#include "opus_types.h"

#define MAX_FRAME_SAMP 5760
#define MAX_PACKET 1500

#define SETUP_BYTE_COUNT 8

#define MAX_DECODES 12

typedef struct {
    int fs;
    int channels;
} TocInfo;

static void ParseToc(const uint8_t *toc, TocInfo *const info) {
    const int samp_freqs[5] = {8000, 12000, 16000, 24000, 48000};
    const int bandwidth = opus_packet_get_bandwidth(toc);

    info->fs = samp_freqs[bandwidth - OPUS_BANDWIDTH_NARROWBAND];
    info->channels = opus_packet_get_nb_channels(toc);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    OpusDecoder *dec;
    opus_int16 *pcm;
    uint8_t *temp_data;
    TocInfo toc;
    int i = 0;
    int err = OPUS_OK;
    int num_decodes = 0;

    if (size < SETUP_BYTE_COUNT + 1) {
        return 0;
    }

    ParseToc(&data[SETUP_BYTE_COUNT], &toc);

    dec = opus_decoder_create(toc.fs, toc.channels, &err);
    if (err != OPUS_OK || dec == NULL) {
        return 0;
    }

    pcm = (opus_int16*) malloc(sizeof(*pcm) * MAX_FRAME_SAMP * toc.channels);

    while (i + SETUP_BYTE_COUNT < size && num_decodes++ < MAX_DECODES) {
        int len, fec;

        len = (opus_uint32) data[i    ] << 24 |
              (opus_uint32) data[i + 1] << 16 |
              (opus_uint32) data[i + 2] <<  8 |
              (opus_uint32) data[i + 3];
        if (len > MAX_PACKET || len < 0 || i + SETUP_BYTE_COUNT + len > size) {
            break;
        }

        fec = data[i + 4] & 1;

        if (len == 0) {

            int frame_size;
            opus_decoder_ctl(dec, OPUS_GET_LAST_PACKET_DURATION(&frame_size));
            (void) opus_decode(dec, NULL, len, pcm, frame_size, fec);
        } else {
            temp_data = (uint8_t*) malloc(len);
            memcpy(temp_data, &data[i + SETUP_BYTE_COUNT], len);

            (void) opus_decode(dec, temp_data, len, pcm, MAX_FRAME_SAMP, fec);

            free(temp_data);
        }

        i += SETUP_BYTE_COUNT + len;
    }

    opus_decoder_destroy(dec);
    free(pcm);

    return 0;
}
