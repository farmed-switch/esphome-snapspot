

#ifndef _V_CODECI_H_
#define _V_CODECI_H_

#define CHUNKSIZE 1024

#include "codebook.h"
#include "ivorbiscodec.h"

#define VI_TRANSFORMB 1
#define VI_WINDOWB 1
#define VI_TIMEB 1
#define VI_FLOORB 2
#define VI_RESB 3
#define VI_MAPB 1

typedef void vorbis_info_floor;

struct vorbis_dsp_state{
  vorbis_info    *vi;
  oggpack_buffer  opb;

  ogg_int32_t   **work;
  ogg_int32_t   **mdctright;
  int             out_begin;
  int             out_end;

  long lW;
  long W;

  ogg_int64_t granulepos;
  ogg_int64_t sequence;
  ogg_int64_t sample_count;

};

extern vorbis_info_floor *floor0_info_unpack(vorbis_info *,oggpack_buffer *);
extern void floor0_free_info(vorbis_info_floor *);
extern int floor0_memosize(vorbis_info_floor *);
extern ogg_int32_t *floor0_inverse1(struct vorbis_dsp_state *,
				    vorbis_info_floor *,ogg_int32_t *);
extern int floor0_inverse2 (struct vorbis_dsp_state *,vorbis_info_floor *,
			    ogg_int32_t *buffer,ogg_int32_t *);

extern vorbis_info_floor *floor1_info_unpack(vorbis_info *,oggpack_buffer *);
extern void floor1_free_info(vorbis_info_floor *);
extern int floor1_memosize(vorbis_info_floor *);
extern ogg_int32_t *floor1_inverse1(struct vorbis_dsp_state *,
				    vorbis_info_floor *,ogg_int32_t *);
extern int floor1_inverse2 (struct vorbis_dsp_state *,vorbis_info_floor *,
			    ogg_int32_t *buffer,ogg_int32_t *);

typedef struct{
  int   order;
  long  rate;
  long  barkmap;

  int   ampbits;
  int   ampdB;

  int   numbooks;
  char  books[16];

} vorbis_info_floor0;

typedef struct{
  char  class_dim;
  char  class_subs;
  unsigned char  class_book;
  unsigned char  class_subbook[8];
} floor1class;

typedef struct{
  floor1class  *class;
  char         *partitionclass;
  ogg_uint16_t *postlist;
  char         *forward_index;
  char         *hineighbor;
  char         *loneighbor;

  int          partitions;
  int          posts;
  int          mult;

} vorbis_info_floor1;

typedef struct vorbis_info_residue{
  int type;
  unsigned char *stagemasks;
  unsigned char *stagebooks;

  long begin;
  long end;

  int           grouping;
  char          partitions;
  unsigned char groupbook;
  char          stages;
} vorbis_info_residue;

extern void res_clear_info(vorbis_info_residue *info);
extern int res_unpack(vorbis_info_residue *info,
		      vorbis_info *vi,oggpack_buffer *opb);
extern int res_inverse(vorbis_dsp_state *,vorbis_info_residue *info,
		       ogg_int32_t **in,int *nonzero,int ch);

typedef struct {
  unsigned char blockflag;
  unsigned char mapping;
} vorbis_info_mode;

typedef struct coupling_step{
  unsigned char mag;
  unsigned char ang;
} coupling_step;

typedef struct submap{
  char floor;
  char residue;
} submap;

typedef struct vorbis_info_mapping{
  int            submaps;

  unsigned char *chmuxlist;
  submap        *submaplist;

  int            coupling_steps;
  coupling_step *coupling;
} vorbis_info_mapping;

extern int mapping_info_unpack(vorbis_info_mapping *,vorbis_info *,
			       oggpack_buffer *);
extern void mapping_clear_info(vorbis_info_mapping *);
extern int mapping_inverse(struct vorbis_dsp_state *,vorbis_info_mapping *);

typedef struct codec_setup_info {

  long blocksizes[2];

  int        modes;
  int        maps;
  int        floors;
  int        residues;
  int        books;

  vorbis_info_mode       *mode_param;
  vorbis_info_mapping    *map_param;
  char                   *floor_type;
  vorbis_info_floor     **floor_param;
  vorbis_info_residue    *residue_param;
  codebook               *book_param;

} codec_setup_info;

extern vorbis_dsp_state *vorbis_dsp_create(vorbis_info *vi);
extern void     vorbis_dsp_destroy(vorbis_dsp_state *v);
extern int      vorbis_dsp_headerin(vorbis_info *vi,vorbis_comment *vc,
				    ogg_packet *op);

extern int      vorbis_dsp_restart(vorbis_dsp_state *v);
extern int      vorbis_dsp_synthesis(vorbis_dsp_state *vd,
				     ogg_packet *op,int decodep);
extern int      vorbis_dsp_pcmout(vorbis_dsp_state *v,
				  ogg_int16_t *pcm,int samples);
extern int      vorbis_dsp_read(vorbis_dsp_state *v,int samples);
extern long     vorbis_packet_blocksize(vorbis_info *vi,ogg_packet *op);

#endif
