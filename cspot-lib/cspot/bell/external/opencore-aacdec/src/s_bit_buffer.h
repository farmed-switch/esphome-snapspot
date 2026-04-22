

#ifndef S_BIT_BUFFER_H
#define S_BIT_BUFFER_H

typedef struct
{
    UChar *char_ptr;
    UInt32 buffered_bits;
    UInt32 buffer_word;
    UInt32 nrBitsRead;
    UInt32 bufferLen;
}
BIT_BUFFER;

typedef BIT_BUFFER *HANDLE_BIT_BUFFER;

#endif

