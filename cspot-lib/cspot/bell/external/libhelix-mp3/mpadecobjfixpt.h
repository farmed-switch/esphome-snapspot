

#ifndef _MPADECOBJFIXPT_H_
#define _MPADECOBJFIXPT_H_

#include "mp3dec.h"

class CMpaDecObj
{
public:
    CMpaDecObj();
    ~CMpaDecObj();

    int     Init_n(unsigned char *pSync,
                   unsigned long ulSize,
                   unsigned char bUseSize=0);

    void    DecodeFrame_v(unsigned char *pSource,
              	          unsigned long *pulSize,
                   	      unsigned char *pPCM,
                       	  unsigned long *pulPCMSize);

    void    DecodeFrame_v(unsigned char *pSource,
              	          unsigned long *pulSize,
                   	      unsigned char *pPCM,
                       	  unsigned long *pulPCMSize,
						  int *errCode);

    void    GetPCMInfo_v(unsigned long &ulSampRate,
                         int &nChannels,
                         int &nBitsPerSample);

    int     GetSamplesPerFrame_n();

    void    SetTrustPackets(unsigned char bTrust) { m_bTrustPackets = bTrust; }

private:
	void *              m_pDec;

	void *				m_pDecL1;
	void *				m_pDecL2;
	HMP3Decoder			m_pDecL3;

	MP3FrameInfo		m_lastMP3FrameInfo;
	unsigned char		m_bUseFrameSize;
        unsigned char           m_bTrustPackets;
};

#endif
