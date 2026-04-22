

#ifndef PS_CONSTANTS_H
#define PS_CONSTANTS_H

#define NO_SUB_QMF_CHANNELS         12
#define NO_QMF_CHANNELS_IN_HYBRID   3
#define NO_QMF_CHANNELS             64
#define NO_ALLPASS_CHANNELS         23
#define NO_DELAY_CHANNELS           (NO_QMF_CHANNELS-NO_ALLPASS_CHANNELS)
#define DELAY_ALLPASS               2
#define SHORT_DELAY_START           12
#define SHORT_DELAY                 1
#define LONG_DELAY                  14
#define NO_QMF_ALLPASS_CHANNELS    (NO_ALLPASS_CHANNELS-NO_QMF_CHANNELS_IN_HYBRID)
#define NO_QMF_ICC_CHANNELS        (NO_QMF_ALLPASS_CHANNELS+NO_DELAY_CHANNELS)
#define HYBRIDGROUPS                8
#define DECAY_CUTOFF                3
#define NO_SERIAL_ALLPASS_LINKS     3
#define MAX_NO_PS_ENV               5
#define NEGATE_IPD_MASK                 ( 0x00001000 )
#define NO_BINS                         ( 20 )
#define NO_HI_RES_BINS                  ( 34 )
#define NO_LOW_RES_BINS                 ( NO_IID_BINS / 2 )
#define NO_IID_BINS                     ( NO_BINS )
#define NO_ICC_BINS                     ( NO_BINS )
#define NO_LOW_RES_IID_BINS             ( NO_LOW_RES_BINS )
#define NO_LOW_RES_ICC_BINS             ( NO_LOW_RES_BINS )
#define SUBQMF_GROUPS                   ( 10 )
#define QMF_GROUPS                      ( 12 )
#define NO_IID_GROUPS                   ( SUBQMF_GROUPS + QMF_GROUPS )
#define NO_IID_STEPS                    ( 7 )
#define NO_IID_STEPS_FINE               ( 15 )
#define NO_ICC_STEPS                    ( 8 )
#define NO_IID_LEVELS                   ( 2 * NO_IID_STEPS + 1 )
#define NO_IID_LEVELS_FINE              ( 2 * NO_IID_STEPS_FINE + 1 )
#define NO_ICC_LEVELS                   ( NO_ICC_STEPS )

#endif
