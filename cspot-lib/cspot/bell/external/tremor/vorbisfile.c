

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "codec_internal.h"
#include "ivorbisfile.h"

#include "os.h"
#include "misc.h"

#define  NOTOPEN   0
#define  PARTOPEN  1
#define  OPENED    2
#define  STREAMSET 3
#define  LINKSET   4
#define  INITSET   5

static long _get_data(OggVorbis_File *vf){
  errno=0;
  if(vf->datasource){
    unsigned char *buffer=ogg_sync_bufferin(vf->oy,CHUNKSIZE);
    long bytes=(vf->callbacks.read_func)(buffer,1,CHUNKSIZE,vf->datasource);
    if(bytes>0)ogg_sync_wrote(vf->oy,bytes);
    if(bytes==0 && errno)return -1;
    return bytes;
  }else
    return 0;
}

static void _seek_helper(OggVorbis_File *vf,ogg_int64_t offset){
  if(vf->datasource){
    (vf->callbacks.seek_func)(vf->datasource, offset, SEEK_SET);
    vf->offset=offset;
    ogg_sync_reset(vf->oy);
  }else{

    return;
  }
}

static ogg_int64_t _get_next_page(OggVorbis_File *vf,ogg_page *og,
				  ogg_int64_t boundary){
  if(boundary>0)boundary+=vf->offset;
  while(1){
    long more;

    if(boundary>0 && vf->offset>=boundary)return OV_FALSE;
    more=ogg_sync_pageseek(vf->oy,og);

    if(more<0){

      vf->offset-=more;
    }else{
      if(more==0){

	if(!boundary)return OV_FALSE;
	{
	  long ret=_get_data(vf);
	  if(ret==0)return OV_EOF;
	  if(ret<0)return OV_EREAD;
	}
      }else{

	ogg_int64_t ret=vf->offset;
	vf->offset+=more;
	return ret;

      }
    }
  }
}

static ogg_int64_t _get_prev_page(OggVorbis_File *vf,ogg_page *og){
  ogg_int64_t begin=vf->offset;
  ogg_int64_t end=begin;
  ogg_int64_t ret;
  ogg_int64_t offset=-1;

  while(offset==-1){
    begin-=CHUNKSIZE;
    if(begin<0)
      begin=0;
    _seek_helper(vf,begin);
    while(vf->offset<end){
      ret=_get_next_page(vf,og,end-vf->offset);
      if(ret==OV_EREAD)return OV_EREAD;
      if(ret<0){
	break;
      }else{
	offset=ret;
      }
    }
  }

  _seek_helper(vf,offset);
  ret=_get_next_page(vf,og,CHUNKSIZE);
  if(ret<0)

    return OV_EFAULT;

  return offset;
}

static int _bisect_forward_serialno(OggVorbis_File *vf,
				    ogg_int64_t begin,
				    ogg_int64_t searched,
				    ogg_int64_t end,
				    ogg_uint32_t currentno,
				    long m){
  ogg_int64_t endsearched=end;
  ogg_int64_t next=end;
  ogg_page og={0,0,0,0};
  ogg_int64_t ret;

  while(searched<endsearched){
    ogg_int64_t bisect;

    if(endsearched-searched<CHUNKSIZE){
      bisect=searched;
    }else{
      bisect=(searched+endsearched)/2;
    }

    _seek_helper(vf,bisect);
    ret=_get_next_page(vf,&og,-1);
    if(ret==OV_EREAD)return OV_EREAD;
    if(ret<0 || ogg_page_serialno(&og)!=currentno){
      endsearched=bisect;
      if(ret>=0)next=ret;
    }else{
      searched=ret+og.header_len+og.body_len;
    }
    ogg_page_release(&og);
  }

  _seek_helper(vf,next);
  ret=_get_next_page(vf,&og,-1);
  if(ret==OV_EREAD)return OV_EREAD;

  if(searched>=end || ret<0){
    ogg_page_release(&og);
    vf->links=m+1;
    vf->offsets=_ogg_malloc((vf->links+1)*sizeof(*vf->offsets));
    vf->serialnos=_ogg_malloc(vf->links*sizeof(*vf->serialnos));
    vf->offsets[m+1]=searched;
  }else{
    ret=_bisect_forward_serialno(vf,next,vf->offset,
				 end,ogg_page_serialno(&og),m+1);
    ogg_page_release(&og);
    if(ret==OV_EREAD)return OV_EREAD;
  }

  vf->offsets[m]=begin;
  vf->serialnos[m]=currentno;
  return 0;
}

static int _decode_clear(OggVorbis_File *vf){
  if(vf->ready_state==INITSET){
    vorbis_dsp_destroy(vf->vd);
    vf->vd=0;
    vf->ready_state=STREAMSET;
  }

  if(vf->ready_state>=STREAMSET){
    vorbis_info_clear(&vf->vi);
    vorbis_comment_clear(&vf->vc);
    vf->ready_state=OPENED;
  }
  return 0;
}

static int _fetch_headers(OggVorbis_File *vf,
			  vorbis_info *vi,
			  vorbis_comment *vc,
			  ogg_uint32_t *serialno,
			  ogg_page *og_ptr){
  ogg_page og={0,0,0,0};
  ogg_packet op={0,0,0,0,0,0};
  int i,ret;

  if(vf->ready_state>OPENED)_decode_clear(vf);

  if(!og_ptr){
    ogg_int64_t llret=_get_next_page(vf,&og,CHUNKSIZE);
    if(llret==OV_EREAD)return OV_EREAD;
    if(llret<0)return OV_ENOTVORBIS;
    og_ptr=&og;
  }

  ogg_stream_reset_serialno(vf->os,ogg_page_serialno(og_ptr));
  if(serialno)*serialno=vf->os->serialno;

  vorbis_info_init(vi);
  vorbis_comment_init(vc);

  i=0;
  while(i<3){
    ogg_stream_pagein(vf->os,og_ptr);
    while(i<3){
      int result=ogg_stream_packetout(vf->os,&op);
      if(result==0)break;
      if(result==-1){
	ret=OV_EBADHEADER;
	goto bail_header;
      }
      if((ret=vorbis_dsp_headerin(vi,vc,&op))){
	goto bail_header;
      }
      i++;
    }
    if(i<3)
      if(_get_next_page(vf,og_ptr,CHUNKSIZE)<0){
	ret=OV_EBADHEADER;
	goto bail_header;
      }
  }

  ogg_packet_release(&op);
  ogg_page_release(&og);
  vf->ready_state=LINKSET;
  return 0;

 bail_header:
  ogg_packet_release(&op);
  ogg_page_release(&og);
  vorbis_info_clear(vi);
  vorbis_comment_clear(vc);
  vf->ready_state=OPENED;

  return ret;
}

static int _set_link_number(OggVorbis_File *vf,int link){
  if(link != vf->current_link) _decode_clear(vf);
  if(vf->ready_state<STREAMSET){
    _seek_helper(vf,vf->offsets[link]);
    ogg_stream_reset_serialno(vf->os,vf->serialnos[link]);
    vf->current_serialno=vf->serialnos[link];
    vf->current_link=link;
    return _fetch_headers(vf,&vf->vi,&vf->vc,&vf->current_serialno,NULL);
  }
  return 0;
}

static int _set_link_number_preserve_pos(OggVorbis_File *vf,int link){
  ogg_int64_t pos=vf->offset;
  int ret=_set_link_number(vf,link);
  if(ret)return ret;
  _seek_helper(vf,pos);
  if(pos<vf->offsets[link] || pos>=vf->offsets[link+1])
    vf->ready_state=STREAMSET;
  return 0;
}

static void _prefetch_all_offsets(OggVorbis_File *vf, ogg_int64_t dataoffset){
  ogg_page og={0,0,0,0};
  int i;
  ogg_int64_t ret;

  vf->dataoffsets=_ogg_malloc(vf->links*sizeof(*vf->dataoffsets));
  vf->pcmlengths=_ogg_malloc(vf->links*2*sizeof(*vf->pcmlengths));

  for(i=0;i<vf->links;i++){
    if(i==0){

      vf->dataoffsets[i]=dataoffset;
      _seek_helper(vf,dataoffset);

    }else{

      _seek_helper(vf,vf->offsets[i]);
      if(_fetch_headers(vf,&vf->vi,&vf->vc,NULL,NULL)<0){
    	vf->dataoffsets[i]=-1;
      }else{
	vf->dataoffsets[i]=vf->offset;
      }
    }

    if(vf->dataoffsets[i]!=-1){
      ogg_int64_t accumulated=0,pos;
      long        lastblock=-1;
      int         result;

      ogg_stream_reset_serialno(vf->os,vf->serialnos[i]);

      while(1){
	ogg_packet op={0,0,0,0,0,0};

	ret=_get_next_page(vf,&og,-1);
	if(ret<0)

	  break;

	if(ogg_page_serialno(&og)!=vf->serialnos[i])
	  break;

	pos=ogg_page_granulepos(&og);

	ogg_stream_pagein(vf->os,&og);
	while((result=ogg_stream_packetout(vf->os,&op))){
	  if(result>0){
	    long thisblock=vorbis_packet_blocksize(&vf->vi,&op);
	    if(lastblock!=-1)
	      accumulated+=(lastblock+thisblock)>>2;
	    lastblock=thisblock;
	  }
	}
	ogg_packet_release(&op);

	if(pos!=-1){

	  accumulated= pos-accumulated;
	  break;
	}
      }

      if(accumulated<0)accumulated=0;

      vf->pcmlengths[i*2]=accumulated;
    }

    {
      ogg_int64_t end=vf->offsets[i+1];
      _seek_helper(vf,end);

      while(1){
	ret=_get_prev_page(vf,&og);
	if(ret<0){

	  vorbis_info_clear(&vf->vi);
	  vorbis_comment_clear(&vf->vc);
	  break;
	}
	if(ogg_page_granulepos(&og)!=-1){
	  vf->pcmlengths[i*2+1]=ogg_page_granulepos(&og)-vf->pcmlengths[i*2];
	  break;
	}
	vf->offset=ret;
      }
    }
  }
  ogg_page_release(&og);
}

static int _make_decode_ready(OggVorbis_File *vf){
  int i;
  switch(vf->ready_state){
  case OPENED:
  case STREAMSET:
    for(i=0;i<vf->links;i++)
      if(vf->offsets[i+1]>=vf->offset)break;
    if(i==vf->links)return -1;
    i=_set_link_number_preserve_pos(vf,i);
    if(i)return i;

  case LINKSET:
    vf->vd=vorbis_dsp_create(&vf->vi);
    vf->ready_state=INITSET;
    vf->bittrack=0;
    vf->samptrack=0;
  case INITSET:
    return 0;
  default:
    return -1;
  }

}

static int _open_seekable2(OggVorbis_File *vf){
  ogg_uint32_t serialno=vf->current_serialno;
  ogg_uint32_t tempserialno;
  ogg_int64_t dataoffset=vf->offset, end;
  ogg_page og={0,0,0,0};

  (vf->callbacks.seek_func)(vf->datasource,0,SEEK_END);
  vf->offset=vf->end=(vf->callbacks.tell_func)(vf->datasource);

  end=_get_prev_page(vf,&og);
  if(end<0)return end;

  tempserialno=ogg_page_serialno(&og);
  ogg_page_release(&og);

  if(tempserialno!=serialno){

    if(_bisect_forward_serialno(vf,0,0,end+1,serialno,0)<0)return OV_EREAD;

  }else{

    if(_bisect_forward_serialno(vf,0,end,end+1,serialno,0))return OV_EREAD;

  }

  _prefetch_all_offsets(vf,dataoffset);
  return ov_raw_seek(vf,0);
}

static int _fetch_and_process_packet(OggVorbis_File *vf,
				     int readp,
				     int spanp){
  ogg_page og={0,0,0,0};
  ogg_packet op={0,0,0,0,0,0};
  int ret=0;

  while(1){

    if(vf->ready_state==INITSET){
      while(1) {
	int result=ogg_stream_packetout(vf->os,&op);
	ogg_int64_t granulepos;

	if(result<0){
	  ret=OV_HOLE;
	  goto cleanup;
	}
	if(result>0){

	  granulepos=op.granulepos;
	  if(!vorbis_dsp_synthesis(vf->vd,&op,1)){

	    vf->samptrack+=vorbis_dsp_pcmout(vf->vd,NULL,0);
	    vf->bittrack+=op.bytes*8;

	    if(granulepos!=-1 && !op.e_o_s){
	      int link=(vf->seekable?vf->current_link:0);
	      int i,samples;

	      if(vf->seekable && link>0)
		granulepos-=vf->pcmlengths[link*2];
	      if(granulepos<0)granulepos=0;

	      samples=vorbis_dsp_pcmout(vf->vd,NULL,0);

	      granulepos-=samples;
	      for(i=0;i<link;i++)
	        granulepos+=vf->pcmlengths[i*2+1];
	      vf->pcm_offset=granulepos;
	    }
	    ret=1;
	    goto cleanup;
	  }
	}
	else
	  break;
      }
    }

    if(vf->ready_state>=OPENED){
      int ret;
      if(!readp){
	ret=0;
	goto cleanup;
      }
      if((ret=_get_next_page(vf,&og,-1))<0){
	ret=OV_EOF;
	goto cleanup;
      }

      vf->bittrack+=og.header_len*8;

      if(vf->ready_state==INITSET){
	if(vf->current_serialno!=ogg_page_serialno(&og)){
	  if(!spanp){
	    ret=OV_EOF;
	    goto cleanup;
	  }

	  _decode_clear(vf);
	}
      }
    }

    if(vf->ready_state!=INITSET){
      int link,ret;

      if(vf->ready_state<STREAMSET){
	if(vf->seekable){
	  vf->current_serialno=ogg_page_serialno(&og);

	  for(link=0;link<vf->links;link++)
	    if(vf->serialnos[link]==vf->current_serialno)break;
	  if(link==vf->links){
	    ret=OV_EBADLINK;
	    goto cleanup;
	  }

	  vf->current_link=link;
	  ret=_fetch_headers(vf,&vf->vi,&vf->vc,&vf->current_serialno,&og);
	  if(ret) goto cleanup;

	}else{

	  int ret=_fetch_headers(vf,&vf->vi,&vf->vc,&vf->current_serialno,&og);
	  if(ret) goto cleanup;
	  vf->current_link++;
	}
      }

      if(_make_decode_ready(vf)) return OV_EBADLINK;
    }
    ogg_stream_pagein(vf->os,&og);
  }
 cleanup:
  ogg_packet_release(&op);
  ogg_page_release(&og);
  return ret;
}

static int _fseek64_wrap(FILE *f,ogg_int64_t off,int whence){
  if(f==NULL)return -1;
  return fseek(f,off,whence);
}

static int _ov_open1(void *f,OggVorbis_File *vf,char *initial,
		     long ibytes, ov_callbacks callbacks){
  int offsettest=(f?callbacks.seek_func(f,0,SEEK_CUR):-1);
  int ret;

  memset(vf,0,sizeof(*vf));

  if( (-1>>1) != -1) return OV_EIMPL;

  vf->datasource=f;
  vf->callbacks = callbacks;

  vf->oy=ogg_sync_create();

  if(initial){
    unsigned char *buffer=ogg_sync_bufferin(vf->oy,ibytes);
    memcpy(buffer,initial,ibytes);
    ogg_sync_wrote(vf->oy,ibytes);
  }

  if(offsettest!=-1)vf->seekable=1;

  vf->links=1;
  vf->os=ogg_stream_create(-1);

  if((ret=_fetch_headers(vf,&vf->vi,&vf->vc,&vf->current_serialno,NULL))<0){
    vf->datasource=NULL;
    ov_clear(vf);
  }else if(vf->ready_state < PARTOPEN)
    vf->ready_state=PARTOPEN;
  return ret;
}

static int _ov_open2(OggVorbis_File *vf){
  if(vf->ready_state < OPENED)
    vf->ready_state=OPENED;
  if(vf->seekable){
    int ret=_open_seekable2(vf);
    if(ret){
      vf->datasource=NULL;
      ov_clear(vf);
    }
    return ret;
  }
  return 0;
}

int ov_clear(OggVorbis_File *vf){
  if(vf){
    vorbis_dsp_destroy(vf->vd);
    vf->vd=0;
    ogg_stream_destroy(vf->os);
    vorbis_info_clear(&vf->vi);
    vorbis_comment_clear(&vf->vc);
    if(vf->dataoffsets)_ogg_free(vf->dataoffsets);
    if(vf->pcmlengths)_ogg_free(vf->pcmlengths);
    if(vf->serialnos)_ogg_free(vf->serialnos);
    if(vf->offsets)_ogg_free(vf->offsets);
    ogg_sync_destroy(vf->oy);

    if(vf->datasource)(vf->callbacks.close_func)(vf->datasource);
    memset(vf,0,sizeof(*vf));
  }
#ifdef DEBUG_LEAKS
  _VDBG_dump();
#endif
  return 0;
}

int ov_open_callbacks(void *f,OggVorbis_File *vf,char *initial,long ibytes,
    ov_callbacks callbacks){
  int ret=_ov_open1(f,vf,initial,ibytes,callbacks);
  if(ret)return ret;
  return _ov_open2(vf);
}

int ov_open(FILE *f,OggVorbis_File *vf,char *initial,long ibytes){
  ov_callbacks callbacks = {
    (size_t (*)(void *, size_t, size_t, void *))  fread,
    (int (*)(void *, ogg_int64_t, int))              _fseek64_wrap,
    (int (*)(void *))                             fclose,
    (long (*)(void *))                            ftell
  };

  return ov_open_callbacks((void *)f, vf, initial, ibytes, callbacks);
}

int ov_test_callbacks(void *f,OggVorbis_File *vf,char *initial,long ibytes,
    ov_callbacks callbacks)
{
  return _ov_open1(f,vf,initial,ibytes,callbacks);
}

int ov_test(FILE *f,OggVorbis_File *vf,char *initial,long ibytes){
  ov_callbacks callbacks = {
    (size_t (*)(void *, size_t, size_t, void *))  fread,
    (int (*)(void *, ogg_int64_t, int))              _fseek64_wrap,
    (int (*)(void *))                             fclose,
    (long (*)(void *))                            ftell
  };

  return ov_test_callbacks((void *)f, vf, initial, ibytes, callbacks);
}

int ov_test_open(OggVorbis_File *vf){
  if(vf->ready_state!=PARTOPEN)return OV_EINVAL;
  return _ov_open2(vf);
}

long ov_streams(OggVorbis_File *vf){
  return vf->links;
}

long ov_seekable(OggVorbis_File *vf){
  return vf->seekable;
}

long ov_bitrate(OggVorbis_File *vf,int i){
  if(vf->ready_state<OPENED)return OV_EINVAL;
  if(i>=vf->links)return OV_EINVAL;
  if(!vf->seekable && i!=0)return ov_bitrate(vf,0);
  if(i<0){
    ogg_int64_t bits=0;
    int i;
    for(i=0;i<vf->links;i++)
      bits+=(vf->offsets[i+1]-vf->dataoffsets[i])*8;

    return bits*1000/ov_time_total(vf,-1);
  }else{
    if(vf->seekable){

      return (vf->offsets[i+1]-vf->dataoffsets[i])*8000/ov_time_total(vf,i);
    }else{

      if(vf->vi.bitrate_nominal>0){
	return vf->vi.bitrate_nominal;
      }else{
	if(vf->vi.bitrate_upper>0){
	  if(vf->vi.bitrate_lower>0){
	    return (vf->vi.bitrate_upper+vf->vi.bitrate_lower)/2;
	  }else{
	    return vf->vi.bitrate_upper;
	  }
	}
	return OV_FALSE;
      }
    }
  }
}

long ov_bitrate_instant(OggVorbis_File *vf){
  long ret;
  if(vf->ready_state<OPENED)return OV_EINVAL;
  if(vf->samptrack==0)return OV_FALSE;
  ret=vf->bittrack/vf->samptrack*vf->vi.rate;
  vf->bittrack=0;
  vf->samptrack=0;
  return ret;
}

long ov_serialnumber(OggVorbis_File *vf,int i){
  if(i>=vf->links)return ov_serialnumber(vf,vf->links-1);
  if(!vf->seekable && i>=0)return ov_serialnumber(vf,-1);
  if(i<0){
    return vf->current_serialno;
  }else{
    return vf->serialnos[i];
  }
}

ogg_int64_t ov_raw_total(OggVorbis_File *vf,int i){
  if(vf->ready_state<OPENED)return OV_EINVAL;
  if(!vf->seekable || i>=vf->links)return OV_EINVAL;
  if(i<0){
    ogg_int64_t acc=0;
    int i;
    for(i=0;i<vf->links;i++)
      acc+=ov_raw_total(vf,i);
    return acc;
  }else{
    return vf->offsets[i+1]-vf->offsets[i];
  }
}

ogg_int64_t ov_pcm_total(OggVorbis_File *vf,int i){
  if(vf->ready_state<OPENED)return OV_EINVAL;
  if(!vf->seekable || i>=vf->links)return OV_EINVAL;
  if(i<0){
    ogg_int64_t acc=0;
    int i;
    for(i=0;i<vf->links;i++)
      acc+=ov_pcm_total(vf,i);
    return acc;
  }else{
    return vf->pcmlengths[i*2+1];
  }
}

ogg_int64_t ov_time_total(OggVorbis_File *vf,int i){
  if(vf->ready_state<OPENED)return OV_EINVAL;
  if(!vf->seekable || i>=vf->links)return OV_EINVAL;
  if(i<0){
    ogg_int64_t acc=0;
    int i;
    for(i=0;i<vf->links;i++)
      acc+=ov_time_total(vf,i);
    return acc;
  }else{
    return ((ogg_int64_t)vf->pcmlengths[i*2+1])*1000/vf->vi.rate;
  }
}

int ov_raw_seek(OggVorbis_File *vf,ogg_int64_t pos){
  ogg_stream_state *work_os=NULL;
  ogg_page og={0,0,0,0};
  ogg_packet op={0,0,0,0,0,0};

  if(vf->ready_state<OPENED)return OV_EINVAL;
  if(!vf->seekable)
    return OV_ENOSEEK;

  if(pos<0 || pos>vf->end)return OV_EINVAL;

  vf->pcm_offset=-1;
  ogg_stream_reset_serialno(vf->os,
			    vf->current_serialno);
  vorbis_dsp_restart(vf->vd);

  _seek_helper(vf,pos);

  {
    int lastblock=0;
    int accblock=0;
    int thisblock;
    int eosflag;

    work_os=ogg_stream_create(vf->current_serialno);
    while(1){
      if(vf->ready_state>=STREAMSET){

	int result=ogg_stream_packetout(work_os,&op);

	if(result>0){

	  if(vf->vi.codec_setup){
	    thisblock=vorbis_packet_blocksize(&vf->vi,&op);
	    if(thisblock<0){
	      ogg_stream_packetout(vf->os,NULL);
	      thisblock=0;
	    }else{

	      if(eosflag)
		ogg_stream_packetout(vf->os,NULL);
	      else
		if(lastblock)accblock+=(lastblock+thisblock)>>2;
	    }

	    if(op.granulepos!=-1){
	      int i,link=vf->current_link;
	      ogg_int64_t granulepos=op.granulepos-vf->pcmlengths[link*2];
	      if(granulepos<0)granulepos=0;

	      for(i=0;i<link;i++)
		granulepos+=vf->pcmlengths[i*2+1];
	      vf->pcm_offset=granulepos-accblock;
	      break;
	    }
	    lastblock=thisblock;
	    continue;
	  }else
	    ogg_stream_packetout(vf->os,NULL);
	}
      }

      if(!lastblock){
	if(_get_next_page(vf,&og,-1)<0){
	  vf->pcm_offset=ov_pcm_total(vf,-1);
	  break;
	}
      }else{

	vf->pcm_offset=-1;
	break;
      }

      if(vf->ready_state>=STREAMSET)
	if(vf->current_serialno!=ogg_page_serialno(&og)){
	  _decode_clear(vf);
	  ogg_stream_destroy(work_os);
	}

      if(vf->ready_state<STREAMSET){
	int link;

	vf->current_serialno=ogg_page_serialno(&og);
	for(link=0;link<vf->links;link++)
	  if(vf->serialnos[link]==vf->current_serialno)break;
	if(link==vf->links)
	  goto seek_error;

	{
	  int ret=_set_link_number_preserve_pos(vf,link);
	  if(ret) goto seek_error;
	}
	ogg_stream_reset_serialno(vf->os,vf->current_serialno);
	ogg_stream_reset_serialno(work_os,vf->current_serialno);

      }

      {
	ogg_page dup;
	ogg_page_dup(&dup,&og);
	eosflag=ogg_page_eos(&og);
	ogg_stream_pagein(vf->os,&og);
	ogg_stream_pagein(work_os,&dup);
      }
    }
  }

  ogg_packet_release(&op);
  ogg_page_release(&og);
  ogg_stream_destroy(work_os);
  vf->bittrack=0;
  vf->samptrack=0;
  return 0;

 seek_error:
  ogg_packet_release(&op);
  ogg_page_release(&og);

  vf->pcm_offset=-1;
  ogg_stream_destroy(work_os);
  _decode_clear(vf);
  return OV_EBADLINK;
}

int ov_pcm_seek_page(OggVorbis_File *vf,ogg_int64_t pos){
  int link=-1;
  ogg_int64_t result=0;
  ogg_int64_t total=ov_pcm_total(vf,-1);
  ogg_page og={0,0,0,0};
  ogg_packet op={0,0,0,0,0,0};

  if(vf->ready_state<OPENED)return OV_EINVAL;
  if(!vf->seekable)return OV_ENOSEEK;
  if(pos<0 || pos>total)return OV_EINVAL;

  for(link=vf->links-1;link>=0;link--){
    total-=vf->pcmlengths[link*2+1];
    if(pos>=total)break;
  }

  if(link!=vf->current_link){
    int ret=_set_link_number(vf,link);
    if(ret) goto seek_error;
  }else{
    vorbis_dsp_restart(vf->vd);
  }

  ogg_stream_reset_serialno(vf->os,vf->serialnos[link]);

  {
    ogg_int64_t end=vf->offsets[link+1];
    ogg_int64_t begin=vf->offsets[link];
    ogg_int64_t begintime = vf->pcmlengths[link*2];
    ogg_int64_t endtime = vf->pcmlengths[link*2+1]+begintime;
    ogg_int64_t target=pos-total+begintime;
    ogg_int64_t best=begin;

    while(begin<end){
      ogg_int64_t bisect;

      if(end-begin<CHUNKSIZE){
	bisect=begin;
      }else{

	bisect=begin +
	  (target-begintime)*(end-begin)/(endtime-begintime) - CHUNKSIZE;
	if(bisect<=begin)
	  bisect=begin+1;
      }

      _seek_helper(vf,bisect);

      while(begin<end){
	result=_get_next_page(vf,&og,end-vf->offset);
	if(result==OV_EREAD) goto seek_error;
	if(result<0){
	  if(bisect<=begin+1)
	    end=begin;
	  else{
	    if(bisect==0) goto seek_error;
	    bisect-=CHUNKSIZE;
	    if(bisect<=begin)bisect=begin+1;
	    _seek_helper(vf,bisect);
	  }
	}else{
	  ogg_int64_t granulepos=ogg_page_granulepos(&og);
	  if(granulepos==-1)continue;
	  if(granulepos<target){
	    best=result;
	    begin=vf->offset;
	    begintime=granulepos;

	    if(target-begintime>44100)break;
	    bisect=begin;
	  }else{
	    if(bisect<=begin+1)
	      end=begin;
	    else{
	      if(end==vf->offset){
		end=result;
		bisect-=CHUNKSIZE;
		if(bisect<=begin)bisect=begin+1;
		_seek_helper(vf,bisect);
	      }else{
		end=result;
		endtime=granulepos;
		break;
	      }
	    }
	  }
	}
      }
    }

    {

      _seek_helper(vf,best);
      vf->pcm_offset=-1;

      if(_get_next_page(vf,&og,-1)<0){
	ogg_page_release(&og);
	return OV_EOF;
      }

      ogg_stream_pagein(vf->os,&og);

      while(1){
	result=ogg_stream_packetpeek(vf->os,&op);
	if(result==0){

	  _seek_helper(vf,best);

	  while(1){
	    result=_get_prev_page(vf,&og);
	    if(result<0) goto seek_error;
	    if(ogg_page_granulepos(&og)>-1 ||
	       !ogg_page_continued(&og)){
	      return ov_raw_seek(vf,result);
	    }
	    vf->offset=result;
	  }
	}
	if(result<0){
	  result = OV_EBADPACKET;
	  goto seek_error;
	}
	if(op.granulepos!=-1){
	  vf->pcm_offset=op.granulepos-vf->pcmlengths[vf->current_link*2];
	  if(vf->pcm_offset<0)vf->pcm_offset=0;
	  vf->pcm_offset+=total;
	  break;
	}else
	  result=ogg_stream_packetout(vf->os,NULL);
      }
    }
  }

  if(vf->pcm_offset>pos || pos>ov_pcm_total(vf,-1)){
    result=OV_EFAULT;
    goto seek_error;
  }
  vf->bittrack=0;
  vf->samptrack=0;

  ogg_page_release(&og);
  ogg_packet_release(&op);
  return 0;

 seek_error:

  ogg_page_release(&og);
  ogg_packet_release(&op);

  vf->pcm_offset=-1;
  _decode_clear(vf);
  return (int)result;
}

int ov_pcm_seek(OggVorbis_File *vf,ogg_int64_t pos){
  ogg_packet op={0,0,0,0,0,0};
  ogg_page og={0,0,0,0};
  int thisblock,lastblock=0;
  int ret=ov_pcm_seek_page(vf,pos);
  if(ret<0)return ret;
  if(_make_decode_ready(vf))return OV_EBADLINK;

  while(1){

    int ret=ogg_stream_packetpeek(vf->os,&op);
    if(ret>0){
      thisblock=vorbis_packet_blocksize(&vf->vi,&op);
      if(thisblock<0){
	ogg_stream_packetout(vf->os,NULL);
	continue;
      }
      if(lastblock)vf->pcm_offset+=(lastblock+thisblock)>>2;

      if(vf->pcm_offset+((thisblock+
			  vorbis_info_blocksize(&vf->vi,1))>>2)>=pos)break;

      ogg_stream_packetout(vf->os,NULL);
      vorbis_dsp_synthesis(vf->vd,&op,0);

      if(op.granulepos>-1){
	int i;

	vf->pcm_offset=op.granulepos-vf->pcmlengths[vf->current_link*2];
	if(vf->pcm_offset<0)vf->pcm_offset=0;
	for(i=0;i<vf->current_link;i++)
	  vf->pcm_offset+=vf->pcmlengths[i*2+1];
      }

      lastblock=thisblock;

    }else{
      if(ret<0 && ret!=OV_HOLE)break;

      if(_get_next_page(vf,&og,-1)<0)break;
      if(vf->current_serialno!=ogg_page_serialno(&og))_decode_clear(vf);

      if(vf->ready_state<STREAMSET){
	int link,ret;

	vf->current_serialno=ogg_page_serialno(&og);
	for(link=0;link<vf->links;link++)
	  if(vf->serialnos[link]==vf->current_serialno)break;
	if(link==vf->links){
	  ogg_page_release(&og);
	  ogg_packet_release(&op);
	  return OV_EBADLINK;
	}

	vf->current_link=link;
	ret=_fetch_headers(vf,&vf->vi,&vf->vc,&vf->current_serialno,&og);
	if(ret) return ret;
	if(_make_decode_ready(vf))return OV_EBADLINK;
	lastblock=0;
      }

      ogg_stream_pagein(vf->os,&og);
    }
  }

  vf->bittrack=0;
  vf->samptrack=0;

  while(vf->pcm_offset<pos){
    ogg_int64_t target=pos-vf->pcm_offset;
    long samples=vorbis_dsp_pcmout(vf->vd,NULL,0);

    if(samples>target)samples=target;
    vorbis_dsp_read(vf->vd,samples);
    vf->pcm_offset+=samples;

    if(samples<target)
      if(_fetch_and_process_packet(vf,1,1)<=0)
	vf->pcm_offset=ov_pcm_total(vf,-1);
  }

  ogg_page_release(&og);
  ogg_packet_release(&op);
  return 0;
}

int ov_time_seek(OggVorbis_File *vf,ogg_int64_t milliseconds){

  int link=-1;
  ogg_int64_t pcm_total=ov_pcm_total(vf,-1);
  ogg_int64_t time_total=ov_time_total(vf,-1);

  if(vf->ready_state<OPENED)return OV_EINVAL;
  if(!vf->seekable)return OV_ENOSEEK;
  if(milliseconds<0 || milliseconds>time_total)return OV_EINVAL;

  for(link=vf->links-1;link>=0;link--){
    pcm_total-=vf->pcmlengths[link*2+1];
    time_total-=ov_time_total(vf,link);
    if(milliseconds>=time_total)break;
  }

  {
    int ret=_set_link_number(vf,link);
    if(ret)return ret;
    return
      ov_pcm_seek(vf,pcm_total+(milliseconds-time_total)*
		  vf->vi.rate/1000);
  }
}

int ov_time_seek_page(OggVorbis_File *vf,ogg_int64_t milliseconds){

  int link=-1;
  ogg_int64_t pcm_total=ov_pcm_total(vf,-1);
  ogg_int64_t time_total=ov_time_total(vf,-1);

  if(vf->ready_state<OPENED)return OV_EINVAL;
  if(!vf->seekable)return OV_ENOSEEK;
  if(milliseconds<0 || milliseconds>time_total)return OV_EINVAL;

  for(link=vf->links-1;link>=0;link--){
    pcm_total-=vf->pcmlengths[link*2+1];
    time_total-=ov_time_total(vf,link);
    if(milliseconds>=time_total)break;
  }

  {
    int ret=_set_link_number(vf,link);
    if(ret)return ret;
    return
      ov_pcm_seek_page(vf,pcm_total+(milliseconds-time_total)*
		       vf->vi.rate/1000);
  }
}

ogg_int64_t ov_raw_tell(OggVorbis_File *vf){
  if(vf->ready_state<OPENED)return OV_EINVAL;
  return vf->offset;
}

ogg_int64_t ov_pcm_tell(OggVorbis_File *vf){
  if(vf->ready_state<OPENED)return OV_EINVAL;
  return vf->pcm_offset;
}

ogg_int64_t ov_time_tell(OggVorbis_File *vf){
  int link=0;
  ogg_int64_t pcm_total=0;
  ogg_int64_t time_total=0;

  if(vf->ready_state<OPENED)return OV_EINVAL;
  if(vf->seekable){
    pcm_total=ov_pcm_total(vf,-1);
    time_total=ov_time_total(vf,-1);

    for(link=vf->links-1;link>=0;link--){
      pcm_total-=vf->pcmlengths[link*2+1];
      time_total-=ov_time_total(vf,link);
      if(vf->pcm_offset>=pcm_total)break;
    }
  }

  return time_total+(1000*vf->pcm_offset-pcm_total)/vf->vi.rate;
}

vorbis_info *ov_info(OggVorbis_File *vf,int link){
  if(vf->seekable){
    if(link>=vf->links)return NULL;
    if(link>=0){
      int ret=_set_link_number_preserve_pos(vf,link);
      if(ret)return NULL;
    }
  }
  return &vf->vi;
}

vorbis_comment *ov_comment(OggVorbis_File *vf,int link){
  if(vf->seekable){
    if(link>=vf->links)return NULL;
    if(link>=0){
      int ret=_set_link_number_preserve_pos(vf,link);
      if(ret)return NULL;
    }
  }
  return &vf->vc;
}

long ov_read(OggVorbis_File *vf,void *buffer,int bytes_req,int *bitstream){

  long samples;
  long channels;

  if(vf->ready_state<OPENED)return OV_EINVAL;

  while(1){
    if(vf->ready_state==INITSET){
      channels=vf->vi.channels;
      samples=vorbis_dsp_pcmout(vf->vd,buffer,(bytes_req>>1)/channels);
      if(samples){
	if(samples>0){
	  vorbis_dsp_read(vf->vd,samples);
	  vf->pcm_offset+=samples;
	  if(bitstream)*bitstream=vf->current_link;
	  return samples*2*channels;
	}
	return samples;
      }
    }

    {
      int ret=_fetch_and_process_packet(vf,1,1);
      if(ret==OV_EOF)
	return 0;
      if(ret<=0)
	return ret;
    }

  }
}
