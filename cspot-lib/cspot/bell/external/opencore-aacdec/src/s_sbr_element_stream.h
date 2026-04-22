

#ifndef S_SBR_ELEMENT_STREAM_H
#define S_SBR_ELEMENT_STREAM_H

#define MAXSBRBYTES 1024

typedef struct
{
    Int32 ElementID;
    Int32 ExtensionType;
    Int32 Payload;
    UChar Data[MAXSBRBYTES];
}
SBR_ELEMENT_STREAM;

#endif

