#ifndef PA_JACK_H
#define PA_JACK_H

#include "portaudio.h"

#ifdef __cplusplus
extern "C" {
#endif

PaError PaJack_SetClientName( const char* name );

PaError PaJack_GetClientName(const char** clientName);

#ifdef __cplusplus
}
#endif

#endif
