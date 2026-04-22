#pragma once

#include <pb.h>

namespace bell::nanopb {
bool encodeString(pb_ostream_t* stream, const pb_field_t* field,
                  void* const* arg);

bool encodeVector(pb_ostream_t* stream, const pb_field_t* field,
                  void* const* arg);

bool encodeBoolean(pb_ostream_t* stream, const pb_field_t* field,
                   void* const* arg);
}