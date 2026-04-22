

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ogg.h"
#include "ivorbiscodec.h"
#include "codebook.h"
#include "misc.h"
#include "os.h"

int _ilog(unsigned int v){
  int ret=0;
  while(v){
    ret++;
    v>>=1;
  }
  return(ret);
}

static ogg_uint32_t decpack(long entry,long used_entry,long quantvals,
			    codebook *b,oggpack_buffer *opb,int maptype){
  ogg_uint32_t ret=0;
  int j;

  switch(b->dec_type){

  case 0:
    return (ogg_uint32_t)entry;

  case 1:
    if(maptype==1){

      for(j=0;j<b->dim;j++){
	ogg_uint32_t off=entry%quantvals;
	entry/=quantvals;
	ret|=((ogg_uint16_t *)(b->q_val))[off]<<(b->q_bits*j);
      }
    }else{
      for(j=0;j<b->dim;j++)
	ret|=oggpack_read(opb,b->q_bits)<<(b->q_bits*j);
    }
    return ret;

  case 2:
    for(j=0;j<b->dim;j++){
      ogg_uint32_t off=entry%quantvals;
      entry/=quantvals;
      ret|=off<<(b->q_pack*j);
    }
    return ret;

  case 3:
    return (ogg_uint32_t)used_entry;

  }
  return 0;
}

static ogg_int32_t _float32_unpack(long val,int *point){
  long   mant=val&0x1fffff;
  int    sign=val&0x80000000;

  *point=((val&0x7fe00000L)>>21)-788;

  if(mant){
    while(!(mant&0x40000000)){
      mant<<=1;
      *point-=1;
    }
    if(sign)mant= -mant;
  }else{
    *point=-9999;
  }
  return mant;
}

static int _determine_node_bytes(long used, int leafwidth){

  if(used<2)
    return 4;

  if(leafwidth==3)leafwidth=4;
  if(_ilog(3*used-6)+1 <= leafwidth*4)
    return leafwidth/2?leafwidth/2:1;
  return leafwidth;
}

static int _determine_leaf_words(int nodeb, int leafwidth){
  if(leafwidth>nodeb)return 2;
  return 1;
}

static int _make_words(char *l,long n,ogg_uint32_t *r,long quantvals,
		       codebook *b, oggpack_buffer *opb,int maptype){
  long i,j,count=0;
  long top=0;
  ogg_uint32_t marker[33];

  if(n<2){
    r[0]=0x80000000;
  }else{
    memset(marker,0,sizeof(marker));

    for(i=0;i<n;i++){
      long length=l[i];
      if(length){
	ogg_uint32_t entry=marker[length];
	long chase=0;
	if(count && !entry)return -1;

	for(j=0;j<length-1;j++){
	  int bit=(entry>>(length-j-1))&1;
	  if(chase>=top){
	    top++;
	    r[chase*2]=top;
	    r[chase*2+1]=0;
	  }else
	    if(!r[chase*2+bit])
	      r[chase*2+bit]=top;
	  chase=r[chase*2+bit];
	}
	{
	  int bit=(entry>>(length-j-1))&1;
	  if(chase>=top){
	    top++;
	    r[chase*2+1]=0;
	  }
	  r[chase*2+bit]= decpack(i,count++,quantvals,b,opb,maptype) |
	    0x80000000;
	}

	for(j=length;j>0;j--){
	  if(marker[j]&1){
	    marker[j]=marker[j-1]<<1;
	    break;
	  }
	  marker[j]++;
	}

	for(j=length+1;j<33;j++)
	  if((marker[j]>>1) == entry){
	    entry=marker[j];
	    marker[j]=marker[j-1]<<1;
	  }else
	    break;
      }
    }
  }

  return 0;
}

static int _make_decode_table(codebook *s,char *lengthlist,long quantvals,
			      oggpack_buffer *opb,int maptype){
  int i;
  ogg_uint32_t *work;

  if(s->dec_nodeb==4){
    s->dec_table=_ogg_malloc((s->used_entries*2+1)*sizeof(*work));

    if(_make_words(lengthlist,s->entries,
		   s->dec_table,quantvals,s,opb,maptype))return 1;

    return 0;
  }

  work=alloca((s->used_entries*2-2)*sizeof(*work));
  if(_make_words(lengthlist,s->entries,work,quantvals,s,opb,maptype))return 1;
  s->dec_table=_ogg_malloc((s->used_entries*(s->dec_leafw+1)-2)*
			   s->dec_nodeb);

  if(s->dec_leafw==1){
    switch(s->dec_nodeb){
    case 1:
      for(i=0;i<s->used_entries*2-2;i++)
	  ((unsigned char *)s->dec_table)[i]=
	    ((work[i] & 0x80000000UL) >> 24) | work[i];
      break;
    case 2:
      for(i=0;i<s->used_entries*2-2;i++)
	  ((ogg_uint16_t *)s->dec_table)[i]=
	    ((work[i] & 0x80000000UL) >> 16) | work[i];
      break;
    }

  }else{

    long top=s->used_entries*3-2;
    if(s->dec_nodeb==1){
      unsigned char *out=(unsigned char *)s->dec_table;

      for(i=s->used_entries*2-4;i>=0;i-=2){
	if(work[i]&0x80000000UL){
	  if(work[i+1]&0x80000000UL){
	    top-=4;
	    out[top]=(work[i]>>8 & 0x7f)|0x80;
	    out[top+1]=(work[i+1]>>8 & 0x7f)|0x80;
	    out[top+2]=work[i] & 0xff;
	    out[top+3]=work[i+1] & 0xff;
	  }else{
	    top-=3;
	    out[top]=(work[i]>>8 & 0x7f)|0x80;
	    out[top+1]=work[work[i+1]*2];
	    out[top+2]=work[i] & 0xff;
	  }
	}else{
	  if(work[i+1]&0x80000000UL){
	    top-=3;
	    out[top]=work[work[i]*2];
	    out[top+1]=(work[i+1]>>8 & 0x7f)|0x80;
	    out[top+2]=work[i+1] & 0xff;
	  }else{
	    top-=2;
	    out[top]=work[work[i]*2];
	    out[top+1]=work[work[i+1]*2];
	  }
	}
	work[i]=top;
      }
    }else{
      ogg_uint16_t *out=(ogg_uint16_t *)s->dec_table;
      for(i=s->used_entries*2-4;i>=0;i-=2){
	if(work[i]&0x80000000UL){
	  if(work[i+1]&0x80000000UL){
	    top-=4;
	    out[top]=(work[i]>>16 & 0x7fff)|0x8000;
	    out[top+1]=(work[i+1]>>16 & 0x7fff)|0x8000;
	    out[top+2]=work[i] & 0xffff;
	    out[top+3]=work[i+1] & 0xffff;
	  }else{
	    top-=3;
	    out[top]=(work[i]>>16 & 0x7fff)|0x8000;
	    out[top+1]=work[work[i+1]*2];
	    out[top+2]=work[i] & 0xffff;
	  }
	}else{
	  if(work[i+1]&0x80000000UL){
	    top-=3;
	    out[top]=work[work[i]*2];
	    out[top+1]=(work[i+1]>>16 & 0x7fff)|0x8000;
	    out[top+2]=work[i+1] & 0xffff;
	  }else{
	    top-=2;
	    out[top]=work[work[i]*2];
	    out[top+1]=work[work[i+1]*2];
	  }
	}
	work[i]=top;
      }
    }
  }

  return 0;
}

long _book_maptype1_quantvals(codebook *b){

  int bits=_ilog(b->entries);
  int vals=b->entries>>((bits-1)*(b->dim-1)/b->dim);

  while(1){
    long acc=1;
    long acc1=1;
    int i;
    for(i=0;i<b->dim;i++){
      acc*=vals;
      acc1*=vals+1;
    }
    if(acc<=b->entries && acc1>b->entries){
      return(vals);
    }else{
      if(acc>b->entries){
        vals--;
      }else{
        vals++;
      }
    }
  }
}

void vorbis_book_clear(codebook *b){

  if(b->q_val)_ogg_free(b->q_val);
  if(b->dec_table)_ogg_free(b->dec_table);

  memset(b,0,sizeof(*b));
}

int vorbis_book_unpack(oggpack_buffer *opb,codebook *s){
  char         *lengthlist=NULL;
  int           quantvals=0;
  long          i,j;
  int           maptype;

  memset(s,0,sizeof(*s));

  if(oggpack_read(opb,24)!=0x564342)goto _eofout;

  s->dim=oggpack_read(opb,16);
  s->entries=oggpack_read(opb,24);
  if(s->entries==-1)goto _eofout;

  switch((int)oggpack_read(opb,1)){
  case 0:

    lengthlist=(char *)alloca(sizeof(*lengthlist)*s->entries);

    if(oggpack_read(opb,1)){

      for(i=0;i<s->entries;i++){
	if(oggpack_read(opb,1)){
	  long num=oggpack_read(opb,5);
	  if(num==-1)goto _eofout;
	  lengthlist[i]=num+1;
	  s->used_entries++;
	  if(num+1>s->dec_maxlength)s->dec_maxlength=num+1;
	}else
	  lengthlist[i]=0;
      }
    }else{

      s->used_entries=s->entries;
      for(i=0;i<s->entries;i++){
	long num=oggpack_read(opb,5);
	if(num==-1)goto _eofout;
	lengthlist[i]=num+1;
	if(num+1>s->dec_maxlength)s->dec_maxlength=num+1;
      }
    }

    break;
  case 1:

    {
      long length=oggpack_read(opb,5)+1;

      s->used_entries=s->entries;
      lengthlist=(char *)alloca(sizeof(*lengthlist)*s->entries);

      for(i=0;i<s->entries;){
	long num=oggpack_read(opb,_ilog(s->entries-i));
	if(num==-1)goto _eofout;
	for(j=0;j<num && i<s->entries;j++,i++)
	  lengthlist[i]=length;
	s->dec_maxlength=length;
	length++;
      }
    }
    break;
  default:

    goto _eofout;
  }

  if((maptype=oggpack_read(opb,4))>0){
    s->q_min=_float32_unpack(oggpack_read(opb,32),&s->q_minp);
    s->q_del=_float32_unpack(oggpack_read(opb,32),&s->q_delp);
    s->q_bits=oggpack_read(opb,4)+1;
    s->q_seq=oggpack_read(opb,1);

    s->q_del>>=s->q_bits;
    s->q_delp+=s->q_bits;
  }

  switch(maptype){
  case 0:

    s->dec_nodeb=_determine_node_bytes(s->used_entries,_ilog(s->entries)/8+1);
    s->dec_leafw=_determine_leaf_words(s->dec_nodeb,_ilog(s->entries)/8+1);
    s->dec_type=0;

    if(_make_decode_table(s,lengthlist,quantvals,opb,maptype)) goto _errout;
    break;

  case 1:

    quantvals=_book_maptype1_quantvals(s);

    {

      long total1=(s->q_bits*s->dim+8)/8;

      long total2=(_ilog(quantvals-1)*s->dim+8)/8+(s->q_bits+7)/8;

      if(total1<=4 && total1<=total2){

	s->q_val=alloca(sizeof(ogg_uint16_t)*quantvals);
	for(i=0;i<quantvals;i++)
	  ((ogg_uint16_t *)s->q_val)[i]=oggpack_read(opb,s->q_bits);

	if(oggpack_eop(opb)){
	  s->q_val=0;
	  goto _eofout;
	}

	s->dec_type=1;
	s->dec_nodeb=_determine_node_bytes(s->used_entries,
					   (s->q_bits*s->dim+8)/8);
	s->dec_leafw=_determine_leaf_words(s->dec_nodeb,
					   (s->q_bits*s->dim+8)/8);
	if(_make_decode_table(s,lengthlist,quantvals,opb,maptype)){
	  s->q_val=0;
	  goto _errout;
	}

	s->q_val=0;

      }else{

	if(s->q_bits<=8){
	  s->q_val=_ogg_malloc(quantvals);
	  for(i=0;i<quantvals;i++)
	    ((unsigned char *)s->q_val)[i]=oggpack_read(opb,s->q_bits);
	}else{
	  s->q_val=_ogg_malloc(quantvals*2);
	  for(i=0;i<quantvals;i++)
	    ((ogg_uint16_t *)s->q_val)[i]=oggpack_read(opb,s->q_bits);
	}

	if(oggpack_eop(opb))goto _eofout;

	s->q_pack=_ilog(quantvals-1);
	s->dec_type=2;
	s->dec_nodeb=_determine_node_bytes(s->used_entries,
					   (_ilog(quantvals-1)*s->dim+8)/8);
	s->dec_leafw=_determine_leaf_words(s->dec_nodeb,
					   (_ilog(quantvals-1)*s->dim+8)/8);
	if(_make_decode_table(s,lengthlist,quantvals,opb,maptype))goto _errout;

      }
    }
    break;
  case 2:

    quantvals=s->entries*s->dim;

    if( (s->q_bits*s->dim+8)/8 <=4){

      s->dec_type=1;
      s->dec_nodeb=_determine_node_bytes(s->used_entries,(s->q_bits*s->dim+8)/8);
      s->dec_leafw=_determine_leaf_words(s->dec_nodeb,(s->q_bits*s->dim+8)/8);
      if(_make_decode_table(s,lengthlist,quantvals,opb,maptype))goto _errout;

    }else{

      s->dec_type=3;
      s->dec_nodeb=_determine_node_bytes(s->used_entries,_ilog(s->used_entries-1)/8+1);
      s->dec_leafw=_determine_leaf_words(s->dec_nodeb,_ilog(s->used_entries-1)/8+1);
      if(_make_decode_table(s,lengthlist,quantvals,opb,maptype))goto _errout;

      s->q_pack=(s->q_bits+7)/8*s->dim;
      s->q_val=_ogg_malloc(s->q_pack*s->used_entries);

      if(s->q_bits<=8){
	for(i=0;i<s->used_entries*s->dim;i++)
	  ((unsigned char *)(s->q_val))[i]=oggpack_read(opb,s->q_bits);
      }else{
	for(i=0;i<s->used_entries*s->dim;i++)
	  ((ogg_uint16_t *)(s->q_val))[i]=oggpack_read(opb,s->q_bits);
      }
    }
    break;
  default:
    goto _errout;
  }

  if(oggpack_eop(opb))goto _eofout;

  return 0;
 _errout:
 _eofout:
  vorbis_book_clear(s);
  return -1;
}

static inline ogg_uint32_t decode_packed_entry_number(codebook *book,
						      oggpack_buffer *b){
  ogg_uint32_t chase=0;
  int  read=book->dec_maxlength;
  long lok = oggpack_look(b,read),i;

  while(lok<0 && read>1)
    lok = oggpack_look(b, --read);

  if(lok<0){
    oggpack_adv(b,1);
    return -1;
  }

  if(book->dec_nodeb==1){
    if(book->dec_leafw==1){

      unsigned char *t=(unsigned char *)book->dec_table;
      for(i=0;i<read;i++){
	chase=t[chase*2+((lok>>i)&1)];
	if(chase&0x80UL)break;
      }
      chase&=0x7fUL;

    }else{

      unsigned char *t=(unsigned char *)book->dec_table;
      for(i=0;i<read;i++){
	int bit=(lok>>i)&1;
	int next=t[chase+bit];
	if(next&0x80){
	  chase= (next<<8) | t[chase+bit+1+(!bit || t[chase]&0x80)];
	  break;
	}
	chase=next;
      }
      chase&=0x7fffUL;
    }

  }else{
    if(book->dec_nodeb==2){
      if(book->dec_leafw==1){

	for(i=0;i<read;i++){
	  chase=((ogg_uint16_t *)(book->dec_table))[chase*2+((lok>>i)&1)];
	  if(chase&0x8000UL)break;
	}
	chase&=0x7fffUL;

      }else{

	ogg_uint16_t *t=(ogg_uint16_t *)book->dec_table;
	for(i=0;i<read;i++){
	  int bit=(lok>>i)&1;
	  int next=t[chase+bit];
	  if(next&0x8000){
	    chase= (next<<16) | t[chase+bit+1+(!bit || t[chase]&0x8000)];
	    break;
	  }
	  chase=next;
	}
	chase&=0x7fffffffUL;
      }

    }else{

      for(i=0;i<read;i++){
	chase=((ogg_uint32_t *)(book->dec_table))[chase*2+((lok>>i)&1)];
	if(chase&0x80000000UL)break;
      }
      chase&=0x7fffffffUL;

    }
  }

  if(i<read){
    oggpack_adv(b,i+1);
    return chase;
  }
  oggpack_adv(b,read+1);
  return(-1);
}

long vorbis_book_decode(codebook *book, oggpack_buffer *b){
  if(book->dec_type)return -1;
 return decode_packed_entry_number(book,b);
}

int decode_map(codebook *s, oggpack_buffer *b, ogg_int32_t *v, int point){
  ogg_uint32_t entry = decode_packed_entry_number(s,b);
  int i;
  if(oggpack_eop(b))return(-1);

  switch(s->dec_type){
  case 1:{

    int mask=(1<<s->q_bits)-1;
    for(i=0;i<s->dim;i++){
      v[i]=entry&mask;
      entry>>=s->q_bits;
    }
    break;
  }
  case 2:{

    int mask=(1<<s->q_pack)-1;
    for(i=0;i<s->dim;i++){
      if(s->q_bits<=8)
	v[i]=((unsigned char *)(s->q_val))[entry&mask];
      else
	v[i]=((ogg_uint16_t *)(s->q_val))[entry&mask];
      entry>>=s->q_pack;
    }
    break;
  }
  case 3:{

    void *ptr=(char*)s->q_val+entry*s->q_pack;

    if(s->q_bits<=8){
      for(i=0;i<s->dim;i++)
	v[i]=((unsigned char *)ptr)[i];
    }else{
      for(i=0;i<s->dim;i++)
	v[i]=((ogg_uint16_t *)ptr)[i];
    }
    break;
  }
  default:
    return -1;
  }

  {
    int shiftM=point-s->q_delp;
    ogg_int32_t add=point-s->q_minp;
    if(add>0)
      add= s->q_min >> add;
    else
      add= s->q_min << -add;

    if(shiftM>0)
      for(i=0;i<s->dim;i++)
	v[i]= add + ((v[i] * s->q_del) >> shiftM);
    else
      for(i=0;i<s->dim;i++)
	v[i]= add + ((v[i] * s->q_del) << -shiftM);

    if(s->q_seq)
      for(i=1;i<s->dim;i++)
	v[i]+=v[i-1];
  }

  return 0;
}

long vorbis_book_decodevs_add(codebook *book,ogg_int32_t *a,
			      oggpack_buffer *b,int n,int point){
  if(book->used_entries>0){
    int step=n/book->dim;
    ogg_int32_t *v = (ogg_int32_t *)alloca(sizeof(*v)*book->dim);
    int i,j,o;

    for (j=0;j<step;j++){
      if(decode_map(book,b,v,point))return -1;
      for(i=0,o=j;i<book->dim;i++,o+=step)
	a[o]+=v[i];
    }
  }
  return 0;
}

long vorbis_book_decodev_add(codebook *book,ogg_int32_t *a,
			     oggpack_buffer *b,int n,int point){
  if(book->used_entries>0){
    ogg_int32_t *v = (ogg_int32_t *)alloca(sizeof(*v)*book->dim);
    int i,j;

    for(i=0;i<n;){
      if(decode_map(book,b,v,point))return -1;
      for (j=0;i<n && j<book->dim;j++)
	a[i++]+=v[j];
    }
  }
  return 0;
}

long vorbis_book_decodev_set(codebook *book,ogg_int32_t *a,
			     oggpack_buffer *b,int n,int point){
  if(book->used_entries>0){
    ogg_int32_t *v = (ogg_int32_t *)alloca(sizeof(*v)*book->dim);
    int i,j;

    for(i=0;i<n;){
      if(decode_map(book,b,v,point))return -1;
      for (j=0;i<n && j<book->dim;j++)
	a[i++]=v[j];
    }
  }else{
    int i;

    for(i=0;i<n;){
      a[i++]=0;
    }
  }

  return 0;
}

long vorbis_book_decodevv_add(codebook *book,ogg_int32_t **a,
			      long offset,int ch,
			      oggpack_buffer *b,int n,int point){
  if(book->used_entries>0){

    ogg_int32_t *v = (ogg_int32_t *)alloca(sizeof(*v)*book->dim);
    long i,j;
    int chptr=0;
    long m=offset+n;

    for(i=offset;i<m;){
      if(decode_map(book,b,v,point))return -1;
      for (j=0;i<m && j<book->dim;j++){
	a[chptr++][i]+=v[j];
	if(chptr==ch){
	  chptr=0;
	  i++;
	}
      }
    }
  }

  return 0;
}
