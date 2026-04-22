

#ifndef RANDOM_DATA_H
#define RANDOM_DATA_H

#include <pb.h>

void random_set_seed(uint32_t seed);
uint32_t random_get_seed();

uint32_t rand_word();

int rand_int(int min, int max);

bool rand_bool();

uint8_t rand_byte();

size_t rand_len(size_t max);

void rand_fill(uint8_t *buf, size_t count);

size_t rand_fill_protobuf(uint8_t *buf, size_t min_bytes, size_t max_bytes, int min_tag);

void rand_mess(uint8_t *buf, size_t count);

void rand_protobuf_noise(uint8_t *buffer, size_t bufsize, size_t *msglen);

#endif
