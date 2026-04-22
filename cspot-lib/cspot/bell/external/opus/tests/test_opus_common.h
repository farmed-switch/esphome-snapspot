

static OPUS_INLINE void deb2_impl(unsigned char *_t,unsigned char **_p,int _k,int _x,int _y)
{
  int i;
  if(_x>2){
     if(_y<3)for(i=0;i<_y;i++)*(--*_p)=_t[i+1];
  }else{
     _t[_x]=_t[_x-_y];
     deb2_impl(_t,_p,_k,_x+1,_y);
     for(i=_t[_x-_y]+1;i<_k;i++){
       _t[_x]=i;
       deb2_impl(_t,_p,_k,_x+1,_x);
     }
  }
}

static OPUS_INLINE void debruijn2(int _k, unsigned char *_res)
{
   unsigned char *p;
   unsigned char *t;
   t=malloc(sizeof(unsigned char)*_k*2);
   memset(t,0,sizeof(unsigned char)*_k*2);
   p=&_res[_k*_k];
   deb2_impl(t,&p,_k,1,1);
   free(t);
}

static opus_uint32 Rz, Rw;
static OPUS_INLINE opus_uint32 fast_rand(void)
{
  Rz=36969*(Rz&65535)+(Rz>>16);
  Rw=18000*(Rw&65535)+(Rw>>16);
  return (Rz<<16)+Rw;
}
static opus_uint32 iseed;

#ifdef __GNUC__
__attribute__((noreturn))
#elif defined(_MSC_VER)
__declspec(noreturn)
#endif
static OPUS_INLINE void _test_failed(const char *file, int line)
{
  fprintf(stderr,"\n ***************************************************\n");
  fprintf(stderr," ***         A fatal error was detected.         ***\n");
  fprintf(stderr," ***************************************************\n");
  fprintf(stderr,"Please report this failure and include\n");
  fprintf(stderr,"'make check SEED=%u fails %s at line %d for %s'\n",iseed,file,line,opus_get_version_string());
  fprintf(stderr,"and any relevant details about your system.\n\n");
#if defined(_MSC_VER)
   _set_abort_behavior( 0, _WRITE_ABORT_MSG);
#endif
  abort();
}
#define test_failed() _test_failed(__FILE__, __LINE__);

void regression_test(void);
