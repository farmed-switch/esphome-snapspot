

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <opus.h>
#include <stdio.h>

#define FRAME_SIZE 960
#define SAMPLE_RATE 48000
#define CHANNELS 2
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 64000

#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)

int main(int argc, char **argv)
{
   char *inFile;
   FILE *fin;
   char *outFile;
   FILE *fout;
   opus_int16 in[FRAME_SIZE*CHANNELS];
   opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
   unsigned char cbits[MAX_PACKET_SIZE];
   int nbBytes;

   OpusEncoder *encoder;
   OpusDecoder *decoder;
   int err;

   if (argc != 3)
   {
      fprintf(stderr, "usage: trivial_example input.pcm output.pcm\n");
      fprintf(stderr, "input and output are 16-bit little-endian raw files\n");
      return EXIT_FAILURE;
   }

   encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &err);
   if (err<0)
   {
      fprintf(stderr, "failed to create an encoder: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }

   err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));
   if (err<0)
   {
      fprintf(stderr, "failed to set bitrate: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   inFile = argv[1];
   fin = fopen(inFile, "rb");
   if (fin==NULL)
   {
      fprintf(stderr, "failed to open input file: %s\n", strerror(errno));
      return EXIT_FAILURE;
   }

   decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
   if (err<0)
   {
      fprintf(stderr, "failed to create decoder: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   outFile = argv[2];
   fout = fopen(outFile, "wb");
   if (fout==NULL)
   {
      fprintf(stderr, "failed to open output file: %s\n", strerror(errno));
      return EXIT_FAILURE;
   }

   while (1)
   {
      int i;
      unsigned char pcm_bytes[MAX_FRAME_SIZE*CHANNELS*2];
      int frame_size;
      size_t samples;

      samples = fread(pcm_bytes, sizeof(short)*CHANNELS, FRAME_SIZE, fin);

      if (samples != FRAME_SIZE)
      {
         break;
      }

      for (i=0;i<CHANNELS*FRAME_SIZE;i++)
      {
         in[i]=pcm_bytes[2*i+1]<<8|pcm_bytes[2*i];
      }

      nbBytes = opus_encode(encoder, in, FRAME_SIZE, cbits, MAX_PACKET_SIZE);
      if (nbBytes<0)
      {
         fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
         return EXIT_FAILURE;
      }

      frame_size = opus_decode(decoder, cbits, nbBytes, out, MAX_FRAME_SIZE, 0);
      if (frame_size<0)
      {
         fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
         return EXIT_FAILURE;
      }

      for(i=0;i<CHANNELS*frame_size;i++)
      {
         pcm_bytes[2*i]=out[i]&0xFF;
         pcm_bytes[2*i+1]=(out[i]>>8)&0xFF;
      }

      fwrite(pcm_bytes, sizeof(short), frame_size*CHANNELS, fout);
   }

   opus_encoder_destroy(encoder);
   opus_decoder_destroy(decoder);
   fclose(fin);
   fclose(fout);
   return EXIT_SUCCESS;
}
