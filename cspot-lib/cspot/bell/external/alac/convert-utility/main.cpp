

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ALACEncoder.h"
#include "ALACDecoder.h"
#include "ALACBitUtilities.h"

#include "CAFFileALAC.h"
#include "EndianPortable.h"

#define kMaxBERSize 5
#define kCAFFdataChunkEditsSize  4

#define kWAVERIFFChunkSize 12
#define kWAVEfmtChunkSize 24
#define kWAVEdataChunkHeaderSize 8

#define VERBOSE 0

int32_t GetInputFormat(FILE * inputFile, AudioFormatDescription * theInputFormat, uint32_t * theFileType);
int32_t SetOutputFormat(AudioFormatDescription theInputFormat, AudioFormatDescription * theOutputFormat);
int32_t FindDataStart(FILE * inputFile, uint32_t inputFileType, int32_t * dataPos, int32_t * dataSize);
int32_t EncodeALAC(FILE * inputFile, FILE * outputFile, AudioFormatDescription theInputFormat, AudioFormatDescription theOutputFormat, int32_t inputDataSize);
int32_t DecodeALAC(FILE * inputFile, FILE * outputFile, AudioFormatDescription theInputFormat, AudioFormatDescription theOutputFormat, int32_t inputDataSize, uint32_t outputFileType);
void GetOutputFileType(char * outputFileName, uint32_t * outputFileType);
ALACChannelLayoutTag GetALACChannelLayoutTag(uint32_t inChannelsPerFrame);

void WriteWAVERIFFChunk(FILE * outputFile);
void WriteWAVEfmtChunk(FILE * outputFile, AudioFormatDescription theOutputFormat);
void WriteWAVEdataChunk(FILE * outputFile);
void WriteWAVEChunkSize(FILE * outputFile, uint32_t numDataBytes);

enum
{
    kTestFormatFlag_16BitSourceData    = 1,
    kTestFormatFlag_20BitSourceData    = 2,
    kTestFormatFlag_24BitSourceData    = 3,
    kTestFormatFlag_32BitSourceData    = 4
};

int32_t main (int32_t argc, char * argv[])
{
    char * inputFileName = argv[1];
    char * outputFileName = argv[2];
    FILE * inputFile = NULL;
    FILE * outputFile = NULL;

	bool malformed = argc < 2;

    for (int32_t i = 1; i < argc; ++i)
	{
		if (strcmp (argv[i], "-h") == 0)
		{
			malformed = true;
		}
		else
		{
			if (argv[i][0] == '-')
            {
				printf ("unknown option: %s\n", argv[i]);
				malformed = true;
			}
			else
			{
                if (inputFile == NULL) inputFile = fopen (inputFileName, "rb");
                if(inputFile == NULL)
                {
                    fprintf(stderr," Cannot open file \"%s\"\n", inputFileName);
                    exit (1);
                }

                if (outputFile == NULL) outputFile = fopen (outputFileName, "w+b");
                if(outputFile == NULL)
                {
                    fprintf(stderr," Cannot open file \"%s\"\n", outputFileName);
                    exit (1);
                }
			}
		}

		if (malformed)
		{
			break;
		}
	}

	if (!malformed)
	{
        printf("Input file: %s\n", inputFileName);
        printf("Output file: %s\n", outputFileName);

        int32_t theError = 0;
        AudioFormatDescription inputFormat;
        AudioFormatDescription outputFormat;
        int32_t inputDataPos = 0, inputDataSize = 0;
        uint32_t inputFileType = 0;
        uint32_t outputFileType = 0;

        theError = GetInputFormat(inputFile, &inputFormat, &inputFileType);
        if (theError)
        {
            fprintf(stderr," Cannot determine what format file \"%s\" is\n", inputFileName);
            exit (1);
        }

        if (inputFileType != 'WAVE' && inputFileType != 'caff')
        {
            fprintf(stderr," File \"%s\" is of an unsupported type\n", outputFileName);
            exit (1);
        }

        if (inputFormat.mFormatID != kALACFormatAppleLossless && inputFormat.mFormatID != kALACFormatLinearPCM)
        {
            fprintf(stderr," File \"%s\'s\" data format is of an unsupported type\n", outputFileName);
            exit (1);
        }

        SetOutputFormat(inputFormat, &outputFormat);

        if (theError)
        {
            fprintf(stderr," Cannot determine what format file \"%s\" is\n", outputFileName);
            exit (1);
        }
        FindDataStart(inputFile, inputFileType, &inputDataPos, &inputDataSize);
        fseek(inputFile, inputDataPos, SEEK_SET);

        if (outputFormat.mFormatID == kALACFormatAppleLossless)
        {

            EncodeALAC(inputFile, outputFile, inputFormat, outputFormat, inputDataSize);
        }
        else
        {

            GetOutputFileType(outputFileName, &outputFileType);

            if (outputFileType == 'WAVE' && outputFormat.mChannelsPerFrame > 2)
            {

                fprintf(stderr," Cannot decode more than two channels to WAVE\n");
                exit (1);
            }
            DecodeALAC(inputFile, outputFile, inputFormat, outputFormat, inputDataSize, outputFileType);
        }
	}

	if (malformed) {
		printf ("Usage:\n");
		printf ("Encode:\n");
		printf ("        alacconvert <input wav or caf file> <output caf file>\n");
		printf ("Decode:\n");
		printf ("        alacconvert <input caf file> <output wav or caf file>\n");
		printf ("\n");
		return 1;
	}

    if (inputFile) fclose(inputFile);
    if (outputFile) fclose(outputFile);

	return 0;
}

int32_t GetInputFormat(FILE * inputFile, AudioFormatDescription * theInputFormat, uint32_t * theFileType)
{

    uint8_t theReadBuffer[20];
    bool done = false;
    uint32_t chunkType = 0;

    fread(theReadBuffer, 1, 4, inputFile);

    if (theReadBuffer[0] == 'c' && theReadBuffer[1] == 'a' && theReadBuffer[2] == 'f'  & theReadBuffer[3] == 'f')
    {

        *theFileType = 'caff';

        done = GetCAFFdescFormat(inputFile, theInputFormat);
    }
    else if (theReadBuffer[0] == 'R' && theReadBuffer[1] == 'I' && theReadBuffer[2] == 'F'  & theReadBuffer[3] == 'F')
    {
        fread(theReadBuffer, 1, 8, inputFile);
        if (theReadBuffer[4] == 'W' && theReadBuffer[5] == 'A' && theReadBuffer[6] == 'V'  & theReadBuffer[7] == 'E')
        {

            *theFileType = 'WAVE';

            while (!done)
            {
                uint32_t theChunkSize = 0, theSampleRate = 0;
                fread(theReadBuffer, 1, 4, inputFile);
                chunkType = ((int32_t)(theReadBuffer[0]) << 24) + ((int32_t)(theReadBuffer[1]) << 16) + ((int32_t)(theReadBuffer[2]) << 8) + theReadBuffer[3];
                switch (chunkType)
                {
                    case 'fmt ':
                        fread(theReadBuffer, 1, 20, inputFile);

                        if (theReadBuffer[4] != 1 || theReadBuffer[5] != 0)
                        {

                            *theFileType = 0;
                            return -1;
                        }
                        theInputFormat->mFormatID = kALACFormatLinearPCM;
                        theInputFormat->mChannelsPerFrame = theReadBuffer[6];
                        theSampleRate = ((int32_t)(theReadBuffer[11]) << 24) + ((int32_t)(theReadBuffer[10]) << 16) + ((int32_t)(theReadBuffer[9]) << 8) + theReadBuffer[8];
                        theInputFormat->mSampleRate = theSampleRate;
                        theInputFormat->mBitsPerChannel = theReadBuffer[18];
                        theInputFormat->mFormatFlags = kALACFormatFlagIsSignedInteger | kALACFormatFlagIsPacked;
                        theInputFormat->mBytesPerPacket = theInputFormat->mBytesPerFrame = (theInputFormat->mBitsPerChannel >> 3) * theInputFormat->mChannelsPerFrame;
                        theInputFormat->mFramesPerPacket = 1;
                        theInputFormat->mReserved = 0;
                        done = true;
                        break;
                    default:

                        fread(theReadBuffer, 1, 4, inputFile);
                        theChunkSize = ((int32_t)(theReadBuffer[3]) << 24) + ((int32_t)(theReadBuffer[2]) << 16) + ((int32_t)(theReadBuffer[1]) << 8) + theReadBuffer[0];
                        fseek(inputFile, theChunkSize, SEEK_CUR);
                        break;
                }
            }
        }
        else
        {
            *theFileType = 0;
            return -1;
        }
    }
    else
    {
        *theFileType = 0;
        return -1;
    }

    if (!done) return -1;

    return 0;
}

int32_t SetOutputFormat(AudioFormatDescription theInputFormat, AudioFormatDescription * theOutputFormat)
{
    if (theInputFormat.mFormatID == kALACFormatLinearPCM)
    {

        theOutputFormat->mFormatID = kALACFormatAppleLossless;
        theOutputFormat->mSampleRate =  theInputFormat.mSampleRate;

        switch(theInputFormat.mBitsPerChannel)
        {
            case 16:
                theOutputFormat->mFormatFlags = kTestFormatFlag_16BitSourceData;
                break;
            case 20:
                theOutputFormat->mFormatFlags = kTestFormatFlag_20BitSourceData;
                break;
            case 24:
                theOutputFormat->mFormatFlags = kTestFormatFlag_24BitSourceData;
                break;
            case 32:
                theOutputFormat->mFormatFlags = kTestFormatFlag_32BitSourceData;
                break;
            default:
                return -1;
                break;
        }

        theOutputFormat->mFramesPerPacket = kALACDefaultFramesPerPacket;
        theOutputFormat->mChannelsPerFrame = theInputFormat.mChannelsPerFrame;

        theOutputFormat->mBytesPerPacket = theOutputFormat->mBytesPerFrame = theOutputFormat->mBitsPerChannel = theOutputFormat->mReserved = 0;
    }
    else
    {

        theOutputFormat->mFormatID = kALACFormatLinearPCM;
        theOutputFormat->mSampleRate =  theInputFormat.mSampleRate;

        switch(theInputFormat.mFormatFlags)
        {
            case kTestFormatFlag_16BitSourceData:
                theOutputFormat->mBitsPerChannel = 16;
                break;
            case kTestFormatFlag_20BitSourceData:
                theOutputFormat->mBitsPerChannel = 20;
                break;
            case kTestFormatFlag_24BitSourceData:
                theOutputFormat->mBitsPerChannel = 24;
                break;
            case kTestFormatFlag_32BitSourceData:
                theOutputFormat->mBitsPerChannel = 32;
                break;
            default:
                return -1;
                break;
        }

        theOutputFormat->mFramesPerPacket = 1;
        theOutputFormat->mChannelsPerFrame = theInputFormat.mChannelsPerFrame;
        theOutputFormat->mBytesPerPacket = theOutputFormat->mBytesPerFrame = theOutputFormat->mBitsPerChannel != 20 ? theInputFormat.mChannelsPerFrame * ((theOutputFormat->mBitsPerChannel) >> 3) : (int32_t)(theInputFormat.mChannelsPerFrame * 2.5 + .5);
        theOutputFormat->mFormatFlags = kALACFormatFlagsNativeEndian;
        theOutputFormat->mReserved = 0;
    }
    return 0;
}

int32_t FindDataStart(FILE * inputFile, uint32_t inputFileType, int32_t * dataPos, int32_t * dataSize)
{

    int32_t currentPosition = ftell(inputFile);
    uint8_t theReadBuffer[12];
    uint32_t chunkType = 0, fileSize = 0, chunkSize = 0;
    bool done = false;

    switch (inputFileType)
    {
        case 'WAVE':
            fseek(inputFile, 0, SEEK_SET);
            fread(theReadBuffer, 1, 8, inputFile);
            fileSize = ((int32_t)(theReadBuffer[7]) << 24) + ((int32_t)(theReadBuffer[6]) << 16) + ((int32_t)(theReadBuffer[5]) << 8) + theReadBuffer[4];
            fseek(inputFile, 12, SEEK_SET);
            while (!done && ((uint32_t)(ftell(inputFile)) < fileSize))
            {
                fread(theReadBuffer, 1, 8, inputFile);
                chunkType = ((int32_t)(theReadBuffer[0]) << 24) + ((int32_t)(theReadBuffer[1]) << 16) + ((int32_t)(theReadBuffer[2]) << 8) + theReadBuffer[3];
                switch(chunkType)
                {
                    case 'data':
                        *dataPos = ftell(inputFile);

                        *dataSize = ((int32_t)(theReadBuffer[7]) << 24) + ((int32_t)(theReadBuffer[6]) << 16) + ((int32_t)(theReadBuffer[5]) << 8) + theReadBuffer[4];
                        done = true;
                        break;
                    default:
                        chunkSize = ((int32_t)(theReadBuffer[7]) << 24) + ((int32_t)(theReadBuffer[6]) << 16) + ((int32_t)(theReadBuffer[5]) << 8) + theReadBuffer[4];
                        fseek(inputFile, chunkSize, SEEK_CUR);
                        break;
                }
            }
            break;
        case 'caff':
            done = FindCAFFDataStart(inputFile, dataPos, dataSize);
            break;
    }

    fseek(inputFile, currentPosition, SEEK_SET);

    if (!done) return -1;

    return 0;
}

int32_t EncodeALAC(FILE * inputFile, FILE * outputFile, AudioFormatDescription theInputFormat, AudioFormatDescription theOutputFormat, int32_t inputDataSize)
{
    int32_t theInputPacketBytes = theInputFormat.mChannelsPerFrame * (theInputFormat.mBitsPerChannel >> 3) * theOutputFormat.mFramesPerPacket;
    int32_t theOutputPacketBytes = theInputPacketBytes + kALACMaxEscapeHeaderBytes;
    int32_t thePacketTableSize = 0, packetTablePos = 0, dataPos = 0, dataSizePos = 0, theBERSize = 0, packetTableSizePos;
    uint8_t * theReadBuffer = (uint8_t *)calloc(theInputPacketBytes, 1);
    uint8_t * theWriteBuffer = (uint8_t *)calloc(theOutputPacketBytes, 1);
    int32_t numBytes = 0;
    uint32_t packetTableBytesLeft = 0;
    int64_t numDataBytes = 0;
    port_CAFPacketTableHeader thePacketTableHeader;
    int32_t inputDataBytesRemaining = inputDataSize;
    uint8_t * theMagicCookie = NULL;
    uint32_t theMagicCookieSize = 0;

    ALACEncoder * theEncoder = new ALACEncoder;

    theEncoder->SetFrameSize(theOutputFormat.mFramesPerPacket);
    theEncoder->InitializeEncoder(theOutputFormat);

    WriteCAFFcaffChunk(outputFile);

    WriteCAFFdescChunk(outputFile, theOutputFormat);

    theMagicCookieSize = theEncoder->GetMagicCookieSize(theOutputFormat.mChannelsPerFrame);
    theMagicCookie = (uint8_t *)calloc(theMagicCookieSize, 1);
    theEncoder->GetMagicCookie(theMagicCookie, &theMagicCookieSize);

    WriteCAFFkukiChunk(outputFile, theMagicCookie, theMagicCookieSize);
    free(theMagicCookie);

    if (theOutputFormat.mChannelsPerFrame > 2)
    {
        WriteCAFFchanChunk(outputFile, GetALACChannelLayoutTag(theOutputFormat.mChannelsPerFrame));
    }

    BuildBasePacketTable(theInputFormat, inputDataSize, &thePacketTableSize, &thePacketTableHeader);
    packetTableBytesLeft = thePacketTableSize;

    uint8_t * thePacketTableEntries = (uint8_t *)calloc (thePacketTableSize, 1);

    thePacketTableSize += kMinCAFFPacketTableHeaderSize;

    WriteCAFFpaktChunkHeader(outputFile, &thePacketTableHeader, thePacketTableSize);

    packetTableSizePos = packetTablePos = ftell(outputFile);
    packetTableSizePos -= (sizeof(int64_t) + kMinCAFFPacketTableHeaderSize);

    thePacketTableSize -= kMinCAFFPacketTableHeaderSize;
    fwrite (thePacketTableEntries, 1, thePacketTableSize, outputFile);
    free(thePacketTableEntries);

    dataSizePos = ftell(outputFile) + sizeof(uint32_t);

    WriteCAFFdataChunk(outputFile);

    dataPos = ftell(outputFile);

    while (theInputPacketBytes <= inputDataBytesRemaining)
    {
        numBytes = fread(theReadBuffer, 1, theInputPacketBytes, inputFile);
#if VERBOSE
        printf ("Read %i bytes\n", numBytes);
#endif
        inputDataBytesRemaining -= numBytes;
		if ((theInputFormat.mFormatFlags & 0x02) != kALACFormatFlagsNativeEndian)
		{
#if VERBOSE
			printf ("Byte Swapping!\n");
#endif
			if (theInputFormat.mBitsPerChannel == 16)
			{
				uint16_t * theShort = (uint16_t *)theReadBuffer;
				for (int32_t i = 0; i < (numBytes >> 1); ++i)
				{
					Swap16(&(theShort[i]));
				}
			}
			else if (theInputFormat.mBitsPerChannel == 32)
			{
				uint32_t * theLong = (uint32_t *)theReadBuffer;
				for (int32_t i = 0; i < (numBytes >> 2); ++i)
				{
					Swap32(&(theLong[i]));
				}
			}
			else
			{
				for (int32_t i = 0; i < numBytes; i += 3)
				{
					Swap24(&(theReadBuffer[i]));
				}
			}
 		}
        theEncoder->Encode(theInputFormat, theInputFormat, theReadBuffer, theWriteBuffer, &numBytes);

        GetBERInteger(numBytes, theReadBuffer, &theBERSize);
        fseek(outputFile, packetTablePos, SEEK_SET);
        fwrite(theReadBuffer, 1, theBERSize, outputFile);
        packetTablePos += theBERSize;
        packetTableBytesLeft -= theBERSize;

        fseek(outputFile, dataPos, SEEK_SET);
        fwrite(theWriteBuffer, 1, numBytes, outputFile);
        dataPos += numBytes;
        numDataBytes += numBytes;
#if VERBOSE
        printf ("Writing %i bytes\n", numBytes);
#endif
    }

    if (inputDataBytesRemaining)
    {
        numBytes = fread(theReadBuffer, 1, inputDataBytesRemaining, inputFile);
#if VERBOSE
        printf ("Last Packet! Read %i bytes\n", numBytes);
#endif
        inputDataBytesRemaining -= numBytes;
		if ((theInputFormat.mFormatFlags & 0x02) != kALACFormatFlagsNativeEndian)
		{
#if VERBOSE
			printf ("Byte Swapping!\n");
#endif
			if (theInputFormat.mBitsPerChannel == 16)
			{
				uint16_t * theShort = (uint16_t *)theReadBuffer;
				for (int32_t i = 0; i < (numBytes >> 1); ++i)
				{
					Swap16(&(theShort[i]));
				}
			}
			else if (theInputFormat.mBitsPerChannel == 32)
			{
				uint32_t * theLong = (uint32_t *)theReadBuffer;
				for (int32_t i = 0; i < (numBytes >> 2); ++i)
				{
					Swap32(&(theLong[i]));
				}
			}
			else
			{
				for (int32_t i = 0; i < numBytes; i += 3)
				{
					Swap24(&(theReadBuffer[i]));
				}
			}
 		}
        theEncoder->Encode(theInputFormat, theInputFormat, theReadBuffer, theWriteBuffer, &numBytes);

        GetBERInteger(numBytes, theReadBuffer, &theBERSize);
        fseek(outputFile, packetTablePos, SEEK_SET);
        fwrite(theReadBuffer, 1, theBERSize, outputFile);
        packetTablePos += theBERSize;
        packetTableBytesLeft -= theBERSize;

        fseek(outputFile, dataPos, SEEK_SET);
        fwrite(theWriteBuffer, 1, numBytes, outputFile);
        dataPos += numBytes;
        numDataBytes += numBytes;
#if VERBOSE
        printf ("Writing %i bytes\n", numBytes);
#endif
    }

    if (packetTableBytesLeft > sizeof(port_CAFChunkHeader))
    {
#if VERBOSE
        printf ("Writing %i free bytes\n", packetTableBytesLeft);
#endif
        fseek(outputFile, packetTablePos, SEEK_SET);
        WriteCAFFfreeChunk(outputFile, packetTableBytesLeft);
        fseek(outputFile, packetTableSizePos, SEEK_SET);
        WriteCAFFChunkSize(outputFile, thePacketTableSize - packetTableBytesLeft + kMinCAFFPacketTableHeaderSize);
    }

    fseek(outputFile, dataSizePos, SEEK_SET);
    numDataBytes += kCAFFdataChunkEditsSize;
#if VERBOSE
    printf ("numDataBytes == %i bytes\n", numDataBytes);
#endif
    WriteCAFFChunkSize(outputFile, numDataBytes);

    delete theEncoder;

    free(theReadBuffer);
    free(theWriteBuffer);

    return 0;
}

int32_t DecodeALAC(FILE * inputFile, FILE * outputFile, AudioFormatDescription theInputFormat, AudioFormatDescription theOutputFormat, int32_t inputDataSize, uint32_t outputFileType)
{
    int32_t theInputPacketBytes = theInputFormat.mChannelsPerFrame * (theOutputFormat.mBitsPerChannel >> 3) * theInputFormat.mFramesPerPacket + kALACMaxEscapeHeaderBytes;
    int32_t theOutputPacketBytes = theInputPacketBytes - kALACMaxEscapeHeaderBytes;
    int32_t thePacketTableSize = 0, packetTablePos = 0, outputDataSizePos = 0, inputDataPos = 0;
    uint8_t * theReadBuffer = (uint8_t *)calloc(theInputPacketBytes, 1);
    uint8_t * theWriteBuffer = (uint8_t *)calloc(theOutputPacketBytes, 1);
    int32_t numBytes = 0;
    int64_t numDataBytes = 0;
    uint32_t numFrames = 0;
    BitBuffer theInputBuffer;
    uint8_t * theMagicCookie = NULL;
    uint32_t theMagicCookieSize = 0;

    ALACDecoder * theDecoder = new ALACDecoder;

    theMagicCookieSize = GetMagicCookieSizeFromCAFFkuki(inputFile);
    theMagicCookie = (uint8_t *)calloc(theMagicCookieSize, 1);
    GetMagicCookieFromCAFFkuki(inputFile, theMagicCookie, &theMagicCookieSize);

    theDecoder->Init(theMagicCookie, theMagicCookieSize);
    free(theMagicCookie);

    BitBufferInit(&theInputBuffer, theReadBuffer, theInputPacketBytes);
    inputDataPos = ftell(inputFile);

    if (outputFileType != 'WAVE')
    {

        WriteCAFFcaffChunk(outputFile);

        WriteCAFFdescChunk(outputFile, theOutputFormat);

        if (theOutputFormat.mChannelsPerFrame > 2)
        {

            WriteCAFFchanChunk(outputFile, CAFFChannelLayoutTags[theOutputFormat.mChannelsPerFrame - 1]);
        }

        outputDataSizePos = ftell(outputFile) + sizeof(uint32_t);

        WriteCAFFdataChunk(outputFile);
    }
    else
    {

        WriteWAVERIFFChunk(outputFile);
        WriteWAVEfmtChunk(outputFile, theOutputFormat);
        WriteWAVEdataChunk(outputFile);
        outputDataSizePos = ftell(outputFile) - sizeof(uint32_t);
    }

    FindCAFFPacketTableStart(inputFile, &packetTablePos, &thePacketTableSize);

    fseek(inputFile, packetTablePos, SEEK_SET);
    numBytes = fread(theReadBuffer, 1, kMaxBERSize, inputFile);

    theInputPacketBytes = ReadBERInteger(theReadBuffer, &numBytes);
    packetTablePos += numBytes;
    fseek(inputFile, inputDataPos, SEEK_SET);
    inputDataPos += theInputPacketBytes;

    while ((theInputPacketBytes > 0) && ((size_t)theInputPacketBytes == fread(theReadBuffer, 1, theInputPacketBytes, inputFile)))
    {
#if VERBOSE
        printf ("Read %i bytes\n", theInputPacketBytes);
#endif
        theDecoder->Decode(&theInputBuffer, theWriteBuffer, theInputFormat.mFramesPerPacket, theInputFormat.mChannelsPerFrame, &numFrames);
        numBytes = numFrames * theOutputFormat.mBytesPerFrame;
#if VERBOSE
        printf ("Writing %i bytes\n", numBytes);
#endif
        fwrite(theWriteBuffer, 1, numBytes, outputFile);
        numDataBytes += numBytes;
        fseek(inputFile, packetTablePos, SEEK_SET);
        numBytes = fread(theReadBuffer, 1, kMaxBERSize, inputFile);

        theInputPacketBytes = ReadBERInteger(theReadBuffer, &numBytes);
#if VERBOSE
        printf ("theInputPacketBytes == %i bytes\n", theInputPacketBytes);
#endif
        packetTablePos += numBytes;
        fseek(inputFile, inputDataPos, SEEK_SET);
        inputDataPos += theInputPacketBytes;
        BitBufferReset(&theInputBuffer);
    }
    if (outputFileType != 'WAVE')
    {

        fseek(outputFile, outputDataSizePos, SEEK_SET);
        numDataBytes += kCAFFdataChunkEditsSize;
#if VERBOSE
        printf ("numDataBytes == %i bytes\n", numDataBytes);
#endif
        WriteCAFFChunkSize(outputFile, numDataBytes);
    }
    else
    {

        fseek(outputFile, outputDataSizePos, SEEK_SET);
        WriteWAVEChunkSize(outputFile, (uint32_t)numDataBytes);

        fseek(outputFile, 4, SEEK_SET);
        WriteWAVEChunkSize(outputFile, numDataBytes + sizeof(outputFileType) + kWAVEdataChunkHeaderSize + kWAVEfmtChunkSize);
    }

    delete theDecoder;

    free(theReadBuffer);
    free(theWriteBuffer);

    return 0;
}

void GetOutputFileType(char * outputFileName, uint32_t * outputFileType)
{
    char * typeStr = strrchr(outputFileName, '.');

    *outputFileType = 'caff';

    if (typeStr != NULL)
    {
        if (strlen(typeStr) == 4)
        {
            if (strcmp(typeStr, ".wav") == 0)
            {
                *outputFileType = 'WAVE';
            }
        }
    }
}

ALACChannelLayoutTag GetALACChannelLayoutTag(uint32_t inChannelsPerFrame)
{
    return ALACChannelLayoutTags[inChannelsPerFrame - 1];
}

void WriteWAVERIFFChunk(FILE * outputFile)
{
    uint8_t theReadBuffer[kWAVERIFFChunkSize] = {'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E'};

    fwrite(theReadBuffer, 1, kWAVERIFFChunkSize, outputFile);
}

void WriteWAVEfmtChunk(FILE * outputFile, AudioFormatDescription theOutputFormat)
{

    uint8_t theBuffer[kWAVEfmtChunkSize] = {'f', 'm', 't', ' ', 16, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t theSampleRate = theOutputFormat.mSampleRate;
    uint32_t theAverageBytesPerSecond = theSampleRate * theOutputFormat.mBytesPerFrame;

    theBuffer[10] = theOutputFormat.mChannelsPerFrame;

    theBuffer[12] = theSampleRate & 0xff;
    theBuffer[13] = (theSampleRate >> 8) & 0xff;
    theBuffer[14] = (theSampleRate >> 16) & 0xff;
    theBuffer[15] = theSampleRate >> 24;

    theBuffer[16] = theAverageBytesPerSecond & 0xff;
    theBuffer[17] = (theAverageBytesPerSecond >> 8) & 0xff;
    theBuffer[18] = (theAverageBytesPerSecond >> 16) & 0xff;
    theBuffer[19] = theAverageBytesPerSecond >> 24;

    theBuffer[20] = theOutputFormat.mBytesPerFrame;

    theBuffer[22] = theOutputFormat.mBitsPerChannel;

    fwrite(theBuffer, 1, kWAVEfmtChunkSize, outputFile);
}

void WriteWAVEdataChunk(FILE * outputFile)
{
    uint8_t theBuffer[kWAVEdataChunkHeaderSize] = {'d', 'a', 't', 'a', 0, 0, 0, 0};

    fwrite(theBuffer, 1, kWAVEdataChunkHeaderSize, outputFile);
}

void WriteWAVEChunkSize(FILE * outputFile, uint32_t numDataBytes)
{
    uint8_t theBuffer[4];

    theBuffer[0] = numDataBytes & 0xff;
    theBuffer[1] = (numDataBytes >> 8) & 0xff;
    theBuffer[2] = (numDataBytes >> 16) & 0xff;
    theBuffer[3] = (numDataBytes >> 24) & 0xff;
    fwrite(theBuffer, 1, 4, outputFile);
}
