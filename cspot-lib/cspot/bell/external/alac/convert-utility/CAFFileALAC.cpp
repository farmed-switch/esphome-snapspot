

#include <stdio.h>
#include "CAFFileALAC.h"
#include "EndianPortable.h"

#define kSizeOfChanAtomPlusChannelLayout 24

int32_t FindCAFFPacketTableStart(FILE * inputFile, int32_t * paktPos, int32_t * paktSize)
{

    int32_t currentPosition = ftell(inputFile);
    uint8_t theReadBuffer[12];
    uint32_t chunkType = 0, chunkSize = 0;
    bool done = false;
    int32_t bytesRead = 8;

    fseek(inputFile, bytesRead, SEEK_SET);
    while (!done && bytesRead > 0)
    {
        bytesRead = fread(theReadBuffer, 1, 12, inputFile);
        chunkType = ((int32_t)(theReadBuffer[0]) << 24) + ((int32_t)(theReadBuffer[1]) << 16) + ((int32_t)(theReadBuffer[2]) << 8) + theReadBuffer[3];
        switch(chunkType)
        {
            case 'pakt':
                *paktPos = ftell(inputFile) + kMinCAFFPacketTableHeaderSize;

                *paktSize = ((int32_t)(theReadBuffer[8]) << 24) + ((int32_t)(theReadBuffer[9]) << 16) + ((int32_t)(theReadBuffer[10]) << 8) + theReadBuffer[11];
                done = true;
                break;
            default:
                chunkSize = ((int32_t)(theReadBuffer[8]) << 24) + ((int32_t)(theReadBuffer[9]) << 16) + ((int32_t)(theReadBuffer[10]) << 8) + theReadBuffer[11];
                fseek(inputFile, chunkSize, SEEK_CUR);
                break;
        }
    }

    fseek(inputFile, currentPosition, SEEK_SET);

    return 0;

}

void WriteCAFFcaffChunk(FILE * outputFile)
{
    uint8_t theReadBuffer[8] = {'c', 'a', 'f', 'f', 0, 1, 0, 0};

    fwrite(theReadBuffer, 1, 8, outputFile);
}

void WriteCAFFdescChunk(FILE * outputFile, AudioFormatDescription theOutputFormat)
{
    port_CAFAudioDescription theDescription;
    uint32_t tempFormatFlags = theOutputFormat.mFormatFlags;
    uint8_t theReadBuffer[12] = {'d', 'e', 's', 'c', 0, 0, 0, 0, 0, 0, 0, 0};

    if (theOutputFormat.mFormatID == kALACFormatLinearPCM)
    {
        if (kALACFormatFlagsNativeEndian > 0)
        {
            tempFormatFlags = 0;
        }
        else
        {
            tempFormatFlags = k_port_CAFLinearPCMFormatFlagIsLittleEndian;
        }
    }

    theDescription.mSampleRate = SwapFloat64NtoB(theOutputFormat.mSampleRate);
    theDescription.mFormatID = Swap32NtoB(theOutputFormat.mFormatID);
    theDescription.mFormatFlags = Swap32NtoB(tempFormatFlags);
    theDescription.mBytesPerPacket = Swap32NtoB(theOutputFormat.mBytesPerPacket);
    theDescription.mFramesPerPacket = Swap32NtoB(theOutputFormat.mFramesPerPacket);
    theDescription.mChannelsPerFrame = Swap32NtoB(theOutputFormat.mChannelsPerFrame);
    theDescription.mBitsPerChannel = Swap32NtoB(theOutputFormat.mBitsPerChannel);

    theReadBuffer[11] = sizeof(port_CAFAudioDescription);
    fwrite(theReadBuffer, 1, 12, outputFile);
    fwrite(&theDescription, 1, sizeof(port_CAFAudioDescription), outputFile);
}

void WriteCAFFdataChunk(FILE * outputFile)
{
    uint8_t theReadBuffer[16] = {'d', 'a', 't', 'a', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

    fwrite(theReadBuffer, 1, 16, outputFile);
}

void WriteCAFFkukiChunk(FILE * outputFile, void * inCookie, uint32_t inCookieSize)
{
    uint8_t thekukiHeaderBuffer[12] = {'k', 'u', 'k', 'i', 0, 0, 0, 0, 0, 0, 0, 0};

    thekukiHeaderBuffer[11] = inCookieSize;
    fwrite(thekukiHeaderBuffer, 1, 12, outputFile);
    fwrite(inCookie, 1, inCookieSize, outputFile);
}

void WriteCAFFChunkSize(FILE * outputFile, int64_t numDataBytes)
{
    uint8_t theBuffer[8];

    theBuffer[0] = (numDataBytes >> 56) & 0xff;
    theBuffer[1] = (numDataBytes >> 48) & 0xff;
    theBuffer[2] = (numDataBytes >> 40) & 0xff;
    theBuffer[3] = (numDataBytes >> 32) & 0xff;
    theBuffer[4] = (numDataBytes >> 24) & 0xff;
    theBuffer[5] = (numDataBytes >> 16) & 0xff;
    theBuffer[6] = (numDataBytes >> 8) & 0xff;
    theBuffer[7] = numDataBytes & 0xff;
    fwrite(theBuffer, 1, 8, outputFile);
}

void WriteCAFFchanChunk(FILE * outputFile, uint32_t inChannelTag)
{
    uint8_t theBuffer[24] = {'c', 'h', 'a', 'n', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    theBuffer[11] = sizeof(ALACAudioChannelLayout);
    theBuffer[12] = inChannelTag >> 24;
    theBuffer[13] = (inChannelTag >> 16) & 0xff;
    theBuffer[14] = (inChannelTag >> 8) & 0xff;
    theBuffer[15] = inChannelTag & 0xff;

    fwrite(theBuffer, 1, 24, outputFile);
}

void WriteCAFFfreeChunk(FILE * outputFile, uint32_t theSize)
{
    uint8_t theBuffer[12] = {'f', 'r', 'e', 'e', 0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t i = 0;
    uint32_t theAdjustedSize = theSize - sizeof(port_CAFChunkHeader);

    if (theSize > theAdjustedSize)
    {
        theBuffer[8] = theAdjustedSize >> 24;
        theBuffer[9] = (theAdjustedSize >> 16) & 0xff;
        theBuffer[10] = (theAdjustedSize >> 8) & 0xff;
        theBuffer[11] = theAdjustedSize & 0xff;
        fwrite(theBuffer, 1, 12, outputFile);

        for (i = 0; i < theAdjustedSize; ++i)
        {
            fwrite(&(theBuffer[4]), 1, 1, outputFile);
        }
    }
}

void WriteCAFFpaktChunkHeader(FILE * outputFile, port_CAFPacketTableHeader * thePacketTableHeader, uint32_t thePacketTableSize)
{
    uint8_t theBuffer[12];

    thePacketTableHeader->mNumberPackets = Swap64NtoB(thePacketTableHeader->mNumberPackets);
    thePacketTableHeader->mNumberValidFrames = Swap64NtoB(thePacketTableHeader->mNumberValidFrames);
    thePacketTableHeader->mPrimingFrames = Swap32NtoB(thePacketTableHeader->mPrimingFrames);
    thePacketTableHeader->mRemainderFrames = Swap32NtoB(thePacketTableHeader->mRemainderFrames);

    theBuffer[0] = 'p';
    theBuffer[1] = 'a';
    theBuffer[2] = 'k';
    theBuffer[3] = 't';
    theBuffer[4] = 0;
    theBuffer[5] = 0;
    theBuffer[6] = 0;
    theBuffer[7] = 0;
    theBuffer[8] = thePacketTableSize >> 24;
    theBuffer[9] = (thePacketTableSize >> 16) & 0xff;
    theBuffer[10] = (thePacketTableSize >> 8) & 0xff;
    theBuffer[11] = thePacketTableSize & 0xff;
    fwrite(theBuffer, 1, 12, outputFile);

    fwrite(thePacketTableHeader, 1, kMinCAFFPacketTableHeaderSize, outputFile);
}

void GetBERInteger(int32_t theOriginalValue, uint8_t * theBuffer, int32_t * theBERSize)
{
    if ((theOriginalValue & 0x7f) == theOriginalValue)
    {
        *theBERSize = 1;
        theBuffer[0] = theOriginalValue;
    }
    else if ((theOriginalValue & 0x3fff) == theOriginalValue)
    {
        *theBERSize = 2;
        theBuffer[0] = theOriginalValue >> 7;
        theBuffer[0] |= 0x80;
        theBuffer[1] = theOriginalValue & 0x7f;
    }
    else if ((theOriginalValue & 0x1fffff) == theOriginalValue)
    {
        *theBERSize = 3;
        theBuffer[0] = theOriginalValue >> 14;
        theBuffer[0] |= 0x80;
        theBuffer[1] = (theOriginalValue >> 7) & 0x7f;
        theBuffer[1] |= 0x80;
        theBuffer[2] = theOriginalValue & 0x7f;
    }
    else if ((theOriginalValue & 0x0fffffff) == theOriginalValue)
    {
        *theBERSize = 4;
        theBuffer[0] = theOriginalValue >> 21;
        theBuffer[0] |= 0x80;
        theBuffer[1] = (theOriginalValue >> 14) & 0x7f;
        theBuffer[1] |= 0x80;
        theBuffer[2] = (theOriginalValue >> 7) & 0x7f;
        theBuffer[2] |= 0x80;
        theBuffer[3] = theOriginalValue & 0x7f;
    }
    else
    {
        *theBERSize = 5;
        theBuffer[0] = theOriginalValue >> 28;
        theBuffer[0] |= 0x80;
        theBuffer[1] = (theOriginalValue >> 21) & 0x7f;
        theBuffer[1] |= 0x80;
        theBuffer[2] = (theOriginalValue >> 14) & 0x7f;
        theBuffer[2] |= 0x80;
        theBuffer[3] = (theOriginalValue >> 7) & 0x7f;
        theBuffer[3] |= 0x80;
        theBuffer[4] = theOriginalValue & 0x7f;
    }
}

uint32_t ReadBERInteger(uint8_t * theInputBuffer, int32_t * ioNumBytes)
{
	uint32_t theAnswer = 0;
	uint8_t theData;
	int32_t size = 0;
    do
	{
		theData = theInputBuffer[size];
		theAnswer = (theAnswer << 7) | (theData & 0x7F);
		if (++size > 5)
		{
			size = 0xFFFFFFFF;
			return 0;
		}
	}
	while(((theData & 0x80) != 0) && (size <= *ioNumBytes));

    *ioNumBytes = size;

	return theAnswer;
}

int32_t BuildBasePacketTable(AudioFormatDescription theInputFormat, int32_t inputDataSize, int32_t * theMaxPacketTableSize, port_CAFPacketTableHeader * thePacketTableHeader)
{
    int32_t theMaxPacketSize = 0, theByteSizeTableEntry = 0;

    thePacketTableHeader->mNumberValidFrames = inputDataSize/((theInputFormat.mBitsPerChannel >> 3) * theInputFormat.mChannelsPerFrame);
    thePacketTableHeader->mNumberPackets = thePacketTableHeader->mNumberValidFrames/kALACDefaultFramesPerPacket;
    thePacketTableHeader->mPrimingFrames = 0;
    thePacketTableHeader->mRemainderFrames = thePacketTableHeader->mNumberValidFrames - thePacketTableHeader->mNumberPackets * kALACDefaultFramesPerPacket;
    thePacketTableHeader->mRemainderFrames = kALACDefaultFramesPerPacket - thePacketTableHeader->mRemainderFrames;
    if (thePacketTableHeader->mRemainderFrames) thePacketTableHeader->mNumberPackets += 1;

    theMaxPacketSize = (theInputFormat.mBitsPerChannel >> 3) * theInputFormat.mChannelsPerFrame * kALACDefaultFramesPerPacket + kALACMaxEscapeHeaderBytes;

    if (theMaxPacketSize < 16384)
    {
        theByteSizeTableEntry = 2;
    }
    else
    {
        theByteSizeTableEntry = 3;
    }
    *theMaxPacketTableSize =  theByteSizeTableEntry * thePacketTableHeader->mNumberPackets;

    return 0;
}

uint32_t GetMagicCookieSizeFromCAFFkuki(FILE * inputFile)
{

    int32_t currentPosition = ftell(inputFile);
    uint8_t theReadBuffer[sizeof(ALACSpecificConfig)];
    uint32_t chunkType = 0, chunkSize = 0;
    bool done = false;
    int32_t bytesRead = sizeof(port_CAFFileHeader);
    uint32_t theCookieSize = 0;

    fseek(inputFile, bytesRead, SEEK_SET);
    while (!done && bytesRead > 0)
    {
        bytesRead = fread(theReadBuffer, 1, 12, inputFile);
        chunkType = ((int32_t)(theReadBuffer[0]) << 24) + ((int32_t)(theReadBuffer[1]) << 16) + ((int32_t)(theReadBuffer[2]) << 8) + theReadBuffer[3];
        switch(chunkType)
        {
            case 'kuki':
            {
                theCookieSize = theReadBuffer[11];
                done = true;
                break;
            }
            default:
                chunkSize = ((int32_t)(theReadBuffer[8]) << 24) + ((int32_t)(theReadBuffer[9]) << 16) + ((int32_t)(theReadBuffer[10]) << 8) + theReadBuffer[11];
                fseek(inputFile, chunkSize, SEEK_CUR);
                break;
        }
    }

    fseek(inputFile, currentPosition, SEEK_SET);

    if (!done) return -1;

    return theCookieSize;

}

int32_t GetMagicCookieFromCAFFkuki(FILE * inputFile, uint8_t * outMagicCookie, uint32_t * ioMagicCookieSize)
{

    int32_t currentPosition = ftell(inputFile);
    uint8_t theReadBuffer[12];
    uint32_t chunkType = 0, chunkSize = 0;
    bool done = false, cookieFound = false;
    int32_t bytesRead = sizeof(port_CAFFileHeader);
    uint32_t theStoredCookieSize = 0;

    fseek(inputFile, bytesRead, SEEK_SET);
    while (!done && bytesRead > 0)
    {
        bytesRead = fread(theReadBuffer, 1, 12, inputFile);
        chunkType = ((int32_t)(theReadBuffer[0]) << 24) + ((int32_t)(theReadBuffer[1]) << 16) + ((int32_t)(theReadBuffer[2]) << 8) + theReadBuffer[3];
        switch(chunkType)
        {
            case 'kuki':
            {
                theStoredCookieSize = theReadBuffer[11];
                if (*ioMagicCookieSize >= theStoredCookieSize)
                {
                    fread(outMagicCookie, 1, theStoredCookieSize, inputFile);
                    *ioMagicCookieSize = theStoredCookieSize;
                    cookieFound = true;
                }
                else
                {
                    *ioMagicCookieSize = 0;
                }
                done = true;
                break;
            }
            default:
                chunkSize = ((int32_t)(theReadBuffer[8]) << 24) + ((int32_t)(theReadBuffer[9]) << 16) + ((int32_t)(theReadBuffer[10]) << 8) + theReadBuffer[11];
                fseek(inputFile, chunkSize, SEEK_CUR);
                break;
        }
    }

    fseek(inputFile, currentPosition, SEEK_SET);

    if (!done || !cookieFound) return -1;

    return 0;
}

bool FindCAFFDataStart(FILE * inputFile, int32_t * dataPos, int32_t * dataSize)
{
    bool done = false;
    int32_t bytesRead = 8;
    uint32_t chunkType = 0, chunkSize = 0;
    uint8_t theBuffer[12];

    fseek(inputFile, bytesRead, SEEK_SET);
    while (!done && bytesRead > 0)
    {
        bytesRead = fread(theBuffer, 1, 12, inputFile);
        chunkType = ((int32_t)(theBuffer[0]) << 24) + ((int32_t)(theBuffer[1]) << 16) + ((int32_t)(theBuffer[2]) << 8) + theBuffer[3];
        switch(chunkType)
        {
            case 'data':
                *dataPos = ftell(inputFile) + sizeof(uint32_t);

                *dataSize = ((int32_t)(theBuffer[8]) << 24) + ((int32_t)(theBuffer[9]) << 16) + ((int32_t)(theBuffer[10]) << 8) + theBuffer[11];
                *dataSize -= 4;
                done = true;
                break;
            default:
                chunkSize = ((int32_t)(theBuffer[8]) << 24) + ((int32_t)(theBuffer[9]) << 16) + ((int32_t)(theBuffer[10]) << 8) + theBuffer[11];
                fseek(inputFile, chunkSize, SEEK_CUR);
                break;
        }
    }
    return done;
}

bool GetCAFFdescFormat(FILE * inputFile, AudioFormatDescription * theInputFormat)
{
    bool done = false;
    uint32_t theChunkSize = 0, theChunkType = 0;
    uint8_t theReadBuffer[32];

    fseek(inputFile, 4, SEEK_CUR);

    while (!done)
    {
        fread(theReadBuffer, 1, 4, inputFile);
        theChunkType = ((int32_t)(theReadBuffer[0]) << 24) + ((int32_t)(theReadBuffer[1]) << 16) + ((int32_t)(theReadBuffer[2]) << 8) + theReadBuffer[3];
        switch (theChunkType)
        {
            case 'desc':
                fseek(inputFile, 8, SEEK_CUR);
                fread(theReadBuffer, 1, sizeof(port_CAFAudioDescription), inputFile);
                theInputFormat->mFormatID = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mFormatID);
                theInputFormat->mChannelsPerFrame = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mChannelsPerFrame);
                theInputFormat->mSampleRate = SwapFloat64BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mSampleRate);
                theInputFormat->mBitsPerChannel = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mBitsPerChannel);
                theInputFormat->mFormatFlags = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mFormatFlags);
                theInputFormat->mBytesPerPacket = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mBytesPerPacket);
                if (theInputFormat->mFormatID == kALACFormatAppleLossless)
                {
                    theInputFormat->mBytesPerFrame = 0;
                }
                else
                {
					theInputFormat->mBytesPerFrame = theInputFormat->mBytesPerPacket;
					if ((theInputFormat->mFormatFlags & 0x02) == 0x02)
					{
						theInputFormat->mFormatFlags &= 0xfffffffc;
					}
					else
					{
						theInputFormat->mFormatFlags |= 0x02;
					}

                }
                theInputFormat->mFramesPerPacket = Swap32BtoN(((port_CAFAudioDescription *)(theReadBuffer))->mFramesPerPacket);
                theInputFormat->mReserved = 0;
                done = true;
                break;
            default:

                fread(theReadBuffer, 1, 8, inputFile);
                theChunkSize = ((int32_t)(theReadBuffer[4]) << 24) + ((int32_t)(theReadBuffer[5]) << 16) + ((int32_t)(theReadBuffer[6]) << 8) + theReadBuffer[7];
                fseek(inputFile, theChunkSize, SEEK_CUR);
                break;
        }
    }
    return done;
}
