

#ifndef S_CRC_BUFFER_H
#define S_CRC_BUFFER_H

typedef struct
{
    unsigned short crcState;
    unsigned short crcMask;
    unsigned short crcPoly;
}
CRC_BUFFER;

typedef CRC_BUFFER *HANDLE_CRC;

#endif

