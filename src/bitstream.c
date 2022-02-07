#include "include/bitstream.h"
#include "include/jpg.h"

extern byte RST0;
extern byte RST7;
extern byte EOI;

Bitstream* InitBitstream() {
    Bitstream* b = (Bitstream*)malloc(sizeof(Bitstream));
    b->size = 0;
    b->total_size = 1024;
    b->data = (byte*)malloc(b->total_size);
    return b;
}

void AddItem(Bitstream* bitstream, byte item) {
    if (bitstream->size >= bitstream->total_size) {
        bitstream->total_size += 128;
        bitstream->data = (byte*)realloc(bitstream->data, bitstream->total_size);
    }
    bitstream->data[bitstream->size] = item;
    bitstream->size++;
}

int ReadBit(BitReader* b) {
    if (b->nextByte >= b->bitstream->size) {
        printf("Ran out of data\n");
        return -1;
    }

    int ret = ((b->bitstream->data[b->nextByte]) >> (7 - b->nextBit)) & 1;
    b->nextBit++;
    if (b->nextBit > 7) {
        b->nextByte++;
        b->nextBit = 0;
    }
    return ret;
}

int ReadBits(BitReader* b, uint8_t length) {
    int ret = 0;
    for (int i = 0; i < length; ++i) {
        int bit = ReadBit(b);
        if (bit == -1) {
            ret = -1;
            return ret;
        }
        ret = (ret << 1) | bit;
    }
    return ret;
}

void Align(BitReader *b) {
    if (b->nextByte >= b->bitstream->size)
        return;
    if (!(b->nextBit == 0)) {
        b->nextBit = 0;
        b->nextByte++;
    }
}

BitReader* InitBitReader(Bitstream* stream) {
    BitReader* b = (BitReader*)malloc(sizeof(BitReader));
    b->nextBit = 0;
    b->nextByte = 0;
    b->bitstream = stream;
    return b;
}

void ReadBitStream(file* fileData, ImageData* header) {
    header->bitsream = InitBitstream();
    byte last;
    byte current = get(fileData);

    while (true) {
        last = current;
        current = get(fileData);

        if (last == 0xFF) {
            if (current == 0x00) {
                //printf("%X ", (int)last);
                AddItem(header->bitsream, last);
                current = get(fileData);
                continue;
            }
            else if (current >= RST0 && current <= RST7) {
                current = get(fileData);
                continue;
            }
            else if (current == 0xFF)
                continue;
            else if (current == EOI)
                break;
            else {
                printf("Invalid marker in bitstream");
                header->valid = false;
                return;
            }
        }
        else {
            //printf("%X ", (int)last);
            AddItem(header->bitsream, last);
        }

    }

}