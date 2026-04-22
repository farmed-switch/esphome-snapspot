

#ifndef PB_COMMON_H_INCLUDED
#define PB_COMMON_H_INCLUDED

#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

bool pb_field_iter_begin(pb_field_iter_t *iter, const pb_msgdesc_t *desc, void *message);

bool pb_field_iter_begin_extension(pb_field_iter_t *iter, pb_extension_t *extension);

bool pb_field_iter_begin_const(pb_field_iter_t *iter, const pb_msgdesc_t *desc, const void *message);
bool pb_field_iter_begin_extension_const(pb_field_iter_t *iter, const pb_extension_t *extension);

bool pb_field_iter_next(pb_field_iter_t *iter);

bool pb_field_iter_find(pb_field_iter_t *iter, uint32_t tag);

bool pb_field_iter_find_extension(pb_field_iter_t *iter);

#ifdef PB_VALIDATE_UTF8

bool pb_validate_utf8(const char *s);
#endif

#ifdef __cplusplus
}
#endif

#endif

