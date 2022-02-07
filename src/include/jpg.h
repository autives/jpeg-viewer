#pragma once
#ifndef JPG_H
#define JPG_H

#include "bitstream.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define PI 3.141599

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define COMPONENT(mcu, index) ((index == 0) ? (mcu.y) : ((index == 1) ? (mcu.cb) : ((index == 2) ? (mcu.cr) : (NULL)))) 
#define marker_length(a) ((get(a) << 8) | (get(a)))

typedef struct ColorComponent {
    byte Xsampling_factor;
    byte Ysampling_factor;

    byte AC_huffman_id;
    byte DC_huffman_id;

    byte quant_table_id;
    bool set;
} ColorComponent;

typedef struct HuffmanTable {
    byte offset[17];
    byte symbols[162];
    uint16_t codes[162];
    bool set;
} HuffmanTable;

typedef struct ImageData {
    bool valid;

    bool table_set[4];
    uint16_t* quant_table[4];

    byte frametype;
    uint16_t width;
    uint16_t height;

    uint8_t num_componenets;
    ColorComponent color_components[3];
    bool SOFset;

    HuffmanTable huffman_tables[8];        //DC tables 0-3 AC tables 4-7

    byte start_of_selection;
    byte end_of_selection;

    byte successive_approx_high;
    byte successive_approx_low;

    uint16_t restart_interval;

    Bitstream* bitsream;

    uint32_t mcu_width;
    uint32_t mcu_height;
    uint32_t mcu_width_padded;
    uint32_t mcu_height_padded;

    byte Xsampling_factor;
    byte Ysampling_factor;
} ImageData;

typedef struct MCU {
    union {
        int y[64];
        int r[64];
    };
    union {
        int cb[64];
        int g[64];
    };
    union {
        int cr[64];
        int b[64];
    };
} MCU;

typedef struct file {
    byte* data;
    uint32_t pointer;
    uint32_t size;
} file;

ImageData* NewImage();
ImageData* ReadHeader(const char *fileName);
void PrintHeader(const ImageData* header);

byte get(file* fileData);
void read(file* fileData, byte* buffer, uint32_t size);

void ReadAPPN(file* fileData, ImageData* header);
void ReadComment(file* fileData, ImageData* header);
void ReadQuantizationTable(file* fileData, ImageData* header);
void ReadBaselineSOF(file* fileData, ImageData* header);
void ReadRestartInterval(file* fileData, ImageData* header);
void ReadHuffmanTable(file* fileData, ImageData* header);
void ReadBitStream(file *fileData, ImageData *header);

void GenerateHuffmanCodes(ImageData *header);
byte GetSymbol(BitReader *b, const HuffmanTable* hTable);
bool DecodeMcuComponent(BitReader *b, int* comp, const HuffmanTable* DChTable, const HuffmanTable* AChTable);
MCU* HuffmanDecoder(const ImageData* header);
void InitConstants();

#endif