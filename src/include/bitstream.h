#pragma once
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t byte;

typedef struct Bitstream {
    byte* data;
    uint32_t total_size;
    uint32_t size;
} Bitstream;

typedef struct bit_reader {
    Bitstream* bitstream;
    uint32_t nextByte;
    uint8_t nextBit;
}BitReader;


void AddItem(Bitstream *bitstream, byte item);
int ReadBit(BitReader *b);
int ReadBits(BitReader *b, uint8_t length);
void Align(BitReader *b);
BitReader* InitBitReader(Bitstream *stream);