

#include <stdio.h>
#include <stdlib.h>
#include "ivorbiscodec.h"
#include "ivorbisfile.h"

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

char pcmout[4096];

int main(){
  OggVorbis_File vf;
  int eof=0;
  int current_section;

#ifdef _WIN32

  _setmode( _fileno( stdin ), _O_BINARY );
  _setmode( _fileno( stdout ), _O_BINARY );
#endif

  if(ov_open(stdin, &vf, NULL, 0) < 0) {
      fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
      exit(1);
  }

  {
    char **ptr=ov_comment(&vf,-1)->user_comments;
    vorbis_info *vi=ov_info(&vf,-1);
    while(*ptr){
      fprintf(stderr,"%s\n",*ptr);
      ++ptr;
    }
    fprintf(stderr,"\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
    fprintf(stderr,"\nDecoded length: %ld samples\n",
	    (long)ov_pcm_total(&vf,-1));
    fprintf(stderr,"Encoded by: %s\n\n",ov_comment(&vf,-1)->vendor);
  }

  while(!eof){
    long ret=ov_read(&vf,pcmout,sizeof(pcmout),&current_section);
    if (ret == 0) {

      eof=1;
    } else if (ret < 0) {

    } else {

      fwrite(pcmout,1,ret,stdout);
    }
  }

  ov_clear(&vf);

  fprintf(stderr,"Done.\n");
  return(0);
}
