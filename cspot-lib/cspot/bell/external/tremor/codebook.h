

#ifndef _V_CODEBOOK_H_
#define _V_CODEBOOK_H_

#include "ogg.h"

typedef struct codebook{
  long  dim;
  long  entries;
  long  used_entries;

  int   dec_maxlength;
  void *dec_table;
  int   dec_nodeb;
  int   dec_leafw;
  int   dec_type;

  ogg_int32_t q_min;
  int         q_minp;
  ogg_int32_t q_del;
  int         q_delp;
  int         q_seq;
  int         q_bits;
  int         q_pack;
  void       *q_val;

} codebook;

extern void vorbis_book_clear(codebook *b);
extern int  vorbis_book_unpack(oggpack_buffer *b,codebook *c);

extern long vorbis_book_decode(codebook *book, oggpack_buffer *b);
extern long vorbis_book_decodevs_add(codebook *book, ogg_int32_t *a,
				     oggpack_buffer *b,int n,int point);
extern long vorbis_book_decodev_set(codebook *book, ogg_int32_t *a,
				    oggpack_buffer *b,int n,int point);
extern long vorbis_book_decodev_add(codebook *book, ogg_int32_t *a,
				    oggpack_buffer *b,int n,int point);
extern long vorbis_book_decodevv_add(codebook *book, ogg_int32_t **a,
				     long off,int ch,
				    oggpack_buffer *b,int n,int point);

extern int _ilog(unsigned int v);

#endif
