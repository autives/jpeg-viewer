#include "include/jpg.h"

extern const uint8_t indices[];

void GenerateHuffmanCodes(ImageData* header) {
    uint16_t code;
    byte count = 0;
    for (int i = 0; i < 8; ++i) {
        code = 0;
        if (!header->huffman_tables[i].set)
            continue;
        for (int j = 0; j < 16; ++j) {
            for (int k = header->huffman_tables[i].offset[j]; k < header->huffman_tables[i].offset[j + 1]; ++k) {
                header->huffman_tables[i].codes[k] = code;
                code++;
            }
            code = code << 1;
        }
    }
}

byte GetSymbol(BitReader *b, const HuffmanTable* hTable) {
    int bit = 0;
    uint16_t code = 0;
    for (int i = 0; i < 16; ++i) {
        bit = ReadBit(b);
        if (bit == -1) {
            break;
        }
        code = (code << 1) | bit;
        for (int j = hTable->offset[i]; j < hTable->offset[i + 1]; ++j) {
            if (code == hTable->codes[j])
                return hTable->symbols[j];
        }
    }
    printf("Code doesn't match\n");
    return -1;
}

bool DecodeMcuComponent(BitReader *b, int* comp, const HuffmanTable* DChTable, const HuffmanTable* AChTable) {
    byte nextSymbol = GetSymbol(b, DChTable);
    if (nextSymbol == 255)
        return false;
    if (nextSymbol > 11) {
        printf("Invalid symbol for DC coefficient\n");
        return false;
    }
    
    int length = nextSymbol & 0x0F;
    int coeff = ReadBits(b, length);    
    if (coeff == -1)
        return false;

    if (length != 0 && coeff < (1 << (length - 1))) {
        coeff -= (1 << length) - 1;
    }
    comp[0] = coeff;

    for (int i = 1; i < 64; ++i) {
        nextSymbol = GetSymbol(b, AChTable);

        if (nextSymbol == 255)
            return false;
        else if (nextSymbol == 0x00) {
            for (; i < 64; ++i)
                comp[indices[i]] = 0;
            return true;
        }

        byte to_skip = nextSymbol >> 4;
        byte to_read = nextSymbol & 0x0F;
        for (int j = 0; j < to_skip; ++j, ++i)
            comp[indices[i]] = 0;
        coeff = ReadBits(b, to_read);
        if (coeff == -1)
            return false;
        if (to_read != 0 && coeff < (1 << (to_read - 1))) {
            coeff -= (1 << to_read) - 1;
        }
        comp[indices[i]] = coeff;
    }
    return true;
}

MCU* HuffmanDecoder(const ImageData* header) {
    MCU* mcus = (MCU*) malloc(sizeof(MCU) * header->mcu_height_padded * header->mcu_width_padded);
    int prev_DC_coeff[3] = { 0 };

    BitReader *b = InitBitReader(header->bitsream);

    for (int y = 0; y < header->mcu_height; y += header->Ysampling_factor) {
        for(int x = 0; x < header->mcu_width; x += header->Xsampling_factor) {
            if (header->restart_interval != 0 && ((y * header->mcu_width_padded + x) % header->restart_interval) == 0) {
                prev_DC_coeff[0] = 0;
                prev_DC_coeff[1] = 0;
                prev_DC_coeff[2] = 0;
                Align(b);
            }

            for (int j = 0; j < header->num_componenets; ++j) {
                for(int v = 0; v < header->color_components[j].Ysampling_factor; ++v){
                    for(int h = 0; h < header->color_components[j].Xsampling_factor; ++h){
                        if (!DecodeMcuComponent(b,
                                                COMPONENT(mcus[(y + v) * header->mcu_width_padded + (x + h)], j),
                                                &header->huffman_tables[header->color_components[j].DC_huffman_id],
                                                &header->huffman_tables[header->color_components[j].AC_huffman_id + 4])) {
                            printf("Failed to decode MCU\n");
                            return NULL;
                        }
                        COMPONENT(mcus[(y + v) * header->mcu_width_padded + (x + h)], j)[0] += prev_DC_coeff[j];
                        prev_DC_coeff[j] = COMPONENT(mcus[(y + v) * header->mcu_width_padded + (x + h)], j)[0];
                    }
                }
            }
        }
    }
    return mcus;
}