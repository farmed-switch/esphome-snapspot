

#include <stdlib.h>
#include "ogg.h"
#include "mdct.h"
#include "ivorbiscodec.h"
#include "codec_internal.h"
#include "misc.h"
#include "window_lookup.h"

int vorbis_dsp_restart(vorbis_dsp_state *v){
  if(!v)return -1;
  {
    vorbis_info *vi=v->vi;
    codec_setup_info *ci;

    if(!vi)return -1;
    ci=vi->codec_setup;
    if(!ci)return -1;

    v->out_end=-1;
    v->out_begin=-1;

    v->granulepos=-1;
    v->sequence=-1;
    v->sample_count=-1;
  }
  return 0;
}

vorbis_dsp_state *vorbis_dsp_create(vorbis_info *vi){
  int i;

  vorbis_dsp_state *v=_ogg_calloc(1,sizeof(*v));
  codec_setup_info *ci=(codec_setup_info *)vi->codec_setup;

  v->vi=vi;

  v->work=(ogg_int32_t **)_ogg_malloc(vi->channels*sizeof(*v->work));
  v->mdctright=(ogg_int32_t **)_ogg_malloc(vi->channels*sizeof(*v->mdctright));
  for(i=0;i<vi->channels;i++){
    v->work[i]=(ogg_int32_t *)_ogg_calloc(1,(ci->blocksizes[1]>>1)*
					  sizeof(*v->work[i]));
    v->mdctright[i]=(ogg_int32_t *)_ogg_calloc(1,(ci->blocksizes[1]>>2)*
					       sizeof(*v->mdctright[i]));
  }

  v->lW=0;
  v->W=0;

  vorbis_dsp_restart(v);
  return v;
}

void vorbis_dsp_destroy(vorbis_dsp_state *v){
  int i;
  if(v){
    vorbis_info *vi=v->vi;

    if(v->work){
      for(i=0;i<vi->channels;i++)
        if(v->work[i])_ogg_free(v->work[i]);
      _ogg_free(v->work);
    }
    if(v->mdctright){
      for(i=0;i<vi->channels;i++)
        if(v->mdctright[i])_ogg_free(v->mdctright[i]);
      _ogg_free(v->mdctright);
    }

    _ogg_free(v);
  }
}

static LOOKUP_T *_vorbis_window(int left){
  switch(left){
  case 32:
    return vwin64;
  case 64:
    return vwin128;
  case 128:
    return vwin256;
  case 256:
    return vwin512;
  case 512:
    return vwin1024;
  case 1024:
    return vwin2048;
  case 2048:
    return vwin4096;
#ifndef LIMIT_TO_64kHz
  case 4096:
    return vwin8192;
#endif
  default:
    return(0);
  }
}

int vorbis_dsp_pcmout(vorbis_dsp_state *v,ogg_int16_t *pcm,int samples){
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=(codec_setup_info *)vi->codec_setup;
  if(v->out_begin>-1 && v->out_begin<v->out_end){
    int n=v->out_end-v->out_begin;
    if(pcm){
      int i;
      if(n>samples)n=samples;
      for(i=0;i<vi->channels;i++)
	mdct_unroll_lap(ci->blocksizes[0],ci->blocksizes[1],
			v->lW,v->W,v->work[i],v->mdctright[i],
			_vorbis_window(ci->blocksizes[0]>>1),
			_vorbis_window(ci->blocksizes[1]>>1),
			pcm+i,vi->channels,
			v->out_begin,v->out_begin+n);
    }
    return(n);
  }
  return(0);
}

int vorbis_dsp_read(vorbis_dsp_state *v,int s){
  if(s && v->out_begin+s>v->out_end)return(OV_EINVAL);
  v->out_begin+=s;
  return(0);
}

long vorbis_packet_blocksize(vorbis_info *vi,ogg_packet *op){
  codec_setup_info     *ci=(codec_setup_info *)vi->codec_setup;
  oggpack_buffer       opb;
  int                  mode;
  int modebits=0;
  int v=ci->modes;

  oggpack_readinit(&opb,op->packet);

  if(oggpack_read(&opb,1)!=0){

    return(OV_ENOTAUDIO);
  }

  while(v>1){
    modebits++;
    v>>=1;
  }

  mode=oggpack_read(&opb,modebits);
  if(mode==-1)return(OV_EBADPACKET);
  return(ci->blocksizes[ci->mode_param[mode].blockflag]);
}

static int ilog(ogg_uint32_t v){
  int ret=0;
  if(v)--v;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

int vorbis_dsp_synthesis(vorbis_dsp_state *vd,ogg_packet *op,int decodep){
  vorbis_info          *vi=vd->vi;
  codec_setup_info     *ci=(codec_setup_info *)vi->codec_setup;
  int                   mode,i;

  oggpack_readinit(&vd->opb,op->packet);

  if(oggpack_read(&vd->opb,1)!=0){

    return OV_ENOTAUDIO ;
  }

  mode=oggpack_read(&vd->opb,ilog(ci->modes));
  if(mode==-1 || mode>=ci->modes) return OV_EBADPACKET;

  vd->lW=vd->W;
  vd->W=ci->mode_param[mode].blockflag;
  for(i=0;i<vi->channels;i++)
    mdct_shift_right(ci->blocksizes[vd->lW],vd->work[i],vd->mdctright[i]);

  if(vd->W){
    int temp;
    oggpack_read(&vd->opb,1);
    temp=oggpack_read(&vd->opb,1);
    if(temp==-1) return OV_EBADPACKET;
  }

  if(decodep){
    mapping_inverse(vd,ci->map_param+ci->mode_param[mode].mapping);

    if(vd->out_begin==-1){
      vd->out_begin=0;
      vd->out_end=0;
    }else{
      vd->out_begin=0;
      vd->out_end=ci->blocksizes[vd->lW]/4+ci->blocksizes[vd->W]/4;
    }
  }

  if(vd->sequence==-1 || vd->sequence+1 != op->packetno-3){

    vd->granulepos=-1;
    vd->sample_count=-1;
  }

  vd->sequence=op->packetno;
  vd->sequence=vd->sequence-3;

  if(vd->sample_count==-1){
    vd->sample_count=0;
  }else{
    vd->sample_count+=
      ci->blocksizes[vd->lW]/4+ci->blocksizes[vd->W]/4;
  }

  if(vd->granulepos==-1){
    if(op->granulepos!=-1){

      vd->granulepos=op->granulepos;

      if(vd->sample_count>vd->granulepos){

	if(op->e_o_s){

	  vd->out_end-=vd->sample_count-vd->granulepos;
	}else{

	  vd->out_begin+=vd->sample_count-vd->granulepos;
	  if(vd->out_begin>vd->out_end)
	    vd->out_begin=vd->out_end;
	}

      }

    }
  }else{
    vd->granulepos+=
      ci->blocksizes[vd->lW]/4+ci->blocksizes[vd->W]/4;
    if(op->granulepos!=-1 && vd->granulepos!=op->granulepos){

      if(vd->granulepos>op->granulepos){
	long extra=vd->granulepos-op->granulepos;

	if(extra)
	  if(op->e_o_s){

	    vd->out_end-=extra;
	  }
      }
      vd->granulepos=op->granulepos;
    }
  }

  return(0);
}
