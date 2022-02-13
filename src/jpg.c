#include "include/jpg.h"

const uint8_t indices[] = { 0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63 };

// Start of Frame markers, non-differential, Huffman coding
const byte SOF0 = 0xC0; // Baseline DCT
const byte SOF1 = 0xC1; // Extended sequential DCT
const byte SOF2 = 0xC2; // Progressive DCT
const byte SOF3 = 0xC3; // Lossless (sequential)

// Start of Frame markers, differential, Huffman coding
const byte SOF5 = 0xC5; // Differential sequential DCT
const byte SOF6 = 0xC6; // Differential progressive DCT
const byte SOF7 = 0xC7; // Differential lossless (sequential)

// Start of Frame markers, non-differential, arithmetic coding
const byte SOF9 = 0xC9; // Extended sequential DCT
const byte SOF10 = 0xCA; // Progressive DCT
const byte SOF11 = 0xCB; // Lossless (sequential)

// Start of Frame markers, differential, arithmetic coding
const byte SOF13 = 0xCD; // Differential sequential DCT
const byte SOF14 = 0xCE; // Differential progressive DCT
const byte SOF15 = 0xCF; // Differential lossless (sequential)

// Define Huffman Table(s)
const byte DHT = 0xC4;

// JPEG extensions
const byte JPG = 0xC8;

// Define Arithmetic Coding Conditioning(s)
const byte DAC = 0xCC;

// Restart interval Markers
const byte RST0 = 0xD0;
const byte RST1 = 0xD1;
const byte RST2 = 0xD2;
const byte RST3 = 0xD3;
const byte RST4 = 0xD4;
const byte RST5 = 0xD5;
const byte RST6 = 0xD6;
const byte RST7 = 0xD7;

// Other Markers
const byte SOI = 0xD8; // Start of Image
const byte EOI = 0xD9; // End of Image
const byte SOS = 0xDA; // Start of Scan
const byte DQT = 0xDB; // Define Quantization Table(s)
const byte DNL = 0xDC; // Define Number of Lines
const byte DRI = 0xDD; // Define Restart Interval
const byte DHP = 0xDE; // Define Hierarchical Progression
const byte EXP = 0xDF; // Expand Reference Component(s)

// APPN Markers
const byte APP0 = 0xE0;
const byte APP1 = 0xE1;
const byte APP2 = 0xE2;
const byte APP3 = 0xE3;
const byte APP4 = 0xE4;
const byte APP5 = 0xE5;
const byte APP6 = 0xE6;
const byte APP7 = 0xE7;
const byte APP8 = 0xE8;
const byte APP9 = 0xE9;
const byte APP10 = 0xEA;
const byte APP11 = 0xEB;
const byte APP12 = 0xEC;
const byte APP13 = 0xED;
const byte APP14 = 0xEE;
const byte APP15 = 0xEF;

// Misc Markers
const byte JPG0 = 0xF0;
const byte JPG1 = 0xF1;
const byte JPG2 = 0xF2;
const byte JPG3 = 0xF3;
const byte JPG4 = 0xF4;
const byte JPG5 = 0xF5;
const byte JPG6 = 0xF6;
const byte JPG7 = 0xF7;
const byte JPG8 = 0xF8;
const byte JPG9 = 0xF9;
const byte JPG10 = 0xFA;
const byte JPG11 = 0xFB;
const byte JPG12 = 0xFC;
const byte JPG13 = 0xFD;
const byte COM = 0xFE;
const byte TEM = 0x01;

ImageData* NewImage() {
    ImageData* header = (ImageData*)malloc(sizeof(ImageData));
    header->num_componenets = 0;
    header->valid = true;
    header->SOFset = false;
    for (int i = 0; i < 4; ++i)
        header->table_set[i] = false;
    header->restart_interval = 0;
    for (int i = 0; i < 3; ++i)
        header->color_components[i].set = false;
    for (int i = 0; i < 8; ++i)
        header->huffman_tables[i].set = false;

    return header;
}

byte get(file* fileData) {
    fileData->pointer++;
    return fileData->data[fileData->pointer - 1];
}

void read(file* fileData, byte* buffer, uint32_t size) {
    for (int i = 0; i < size; ++i)
        buffer[i] = fileData->data[fileData->pointer + i];
    fileData->pointer += size;
}

void ReadAPPN(file* fileData, ImageData* header) {
    printf("Reading APPN...\n");
    uint16_t length = marker_length(fileData) - 2;
    byte* ignore = (byte*) malloc(length);
    read(fileData, ignore, length);
    free(ignore);
}

void ReadComment(file* fileData, ImageData* header) {
    printf("Reading Comment...\n");
    uint16_t length = marker_length(fileData) - 2;
    byte* comment = (byte*) malloc(length);
    read(fileData, comment, length);
    free(comment);
}

void ReadQuantizationTable(file* fileData, ImageData* header) {
    printf("Reading Quantization Table...\n");
    int length = marker_length(fileData) - 2;
    uint8_t index;
    uint8_t table_length = 0;

    while (length > 0) {
        byte info = get(fileData);
        index = info & 0x0F;

        header->quant_table[index] = (uint16_t*)malloc(sizeof(uint16_t) * 64);

        if (index > 3) {
            printf("Invalid quantizaion table id: %d\n", index);
            header->valid = false;
            return;
        }

        header->table_set[index] = true;
        if ((info >> 4) == 0) {
            table_length = 64;
            for (int i = 0; i < 64; i++)
                header->quant_table[index][indices[i]] = get(fileData);
            length -= 64 + 1;
        }
        else {
            table_length = 128;
            for (int i = 0; i < 128; i++)
                header->quant_table[index][indices[i]] = marker_length(fileData);
            length -= 128 + 1;
        }

        // for(int i = 0; i < table_length; i ++) {
        //     if(i > 0 && i % 8 == 0)
        //         std::cout<<std::endl;
        //     std::cout<<header->quant_table[index][i]<<" ";
        // }
        // std::cout<<"\n\n";
    }

    if (length != 0) {
        printf("Invalid marker size.\n");
        header->valid = false;
        return;
    }
}

void ReadBaselineSOF(file* fileData, ImageData* header) {
    printf("Reading Start of Frame...\n");
    if (header->SOFset) {
        printf("Multiple SOF markers in single file.\n");
        header->valid = false;
        return;
    }
    header->frametype = SOF0;
    header->SOFset = true;

    uint16_t length = marker_length(fileData) - 2;

    if (get(fileData) != 8) {    //precision
        printf("Invalid precision of baseline DCT.\n");
        header->valid = false;
        return;
    }

    header->height = marker_length(fileData);
    header->width = marker_length(fileData);
    if (header->height == 0 || header->width == 0) {
        printf("Size of image cannot be zero.\n");
        header->valid = false;
        return;
    }
    header->mcu_width = (header->width + 7) / 8;
    header->mcu_height = (header->height + 7) / 8;
    header->mcu_height_padded = header->mcu_height;
    header->mcu_width_padded = header->mcu_width;

    header->num_componenets = get(fileData);
    if (header->num_componenets > 3 || header->num_componenets == 0) {
        printf("Invalid number of components: %d.\n", (int)header->num_componenets);
        header->valid = false;
        return;
    }
    for (int i = 0; i < header->num_componenets; ++i) {
        byte index = get(fileData);
        bool begin_with_zero = false;

        if (index == 0)
            begin_with_zero = true;
        if (begin_with_zero)
            index++;

        if (index == 0 || index > 3) {
            printf("Unsupported color component: %d\n", (unsigned int)index);
            header->valid = false;
            return;
        }

        index--;
        if (header->color_components[index].set) {
            printf("Multiple definitions of same color component: %d", (int)index);
            header->valid = false;
            return;
        }
        header->color_components[index].set = true;

        byte sampling_factor = get(fileData);
        byte Xsampling_factor = sampling_factor >> 4;
        byte Ysampling_factor = sampling_factor & 0x0F;
        if(index == 0) {
            if((Xsampling_factor != 1 && Xsampling_factor != 2)
                || (Ysampling_factor != 1 && Ysampling_factor != 2)) {
                    printf("This sampling factor is not supported in luminance: %d, %d\n", Xsampling_factor, Ysampling_factor);
                    header->valid = false;
                    return;
            }
            if(Xsampling_factor == 2 && (header->mcu_width % 2) == 1)
                header->mcu_width_padded++;
            if(Ysampling_factor == 2 && (header->mcu_height % 2) == 1)
                header->mcu_height_padded++;
            
            header->Xsampling_factor = Xsampling_factor;
            header->Ysampling_factor = Ysampling_factor;
        }
        else{
            if(Xsampling_factor != 1 || Ysampling_factor != 1) {
                printf("This sampling factor is not supported in chrominance: %d, %d\n", Xsampling_factor, Ysampling_factor);
                header->valid = false;
                return;
            }
        }
        header->color_components[index].Xsampling_factor = Xsampling_factor;
        header->color_components[index].Ysampling_factor = Ysampling_factor;

        header->color_components[index].quant_table_id = get(fileData);
        if (header->color_components[index].quant_table_id > 3) {
            printf("Invalid quantization table ID for color component %d.\n", (int)index + 1);
            header->valid = false;
            return;
        }
    }

    if (length - 1 - 4 - 1 - header->num_componenets * (1 + 1 + 1) != 0) {
        printf("Invalid marker length.\n");
        header->valid = false;
        return;
    }

}

void ReadRestartInterval(file* fileData, ImageData* header) {
    printf("Reading restart interval...\n");
    if (marker_length(fileData) != 0x04) {
        printf("Invalid size for restart interval marker.\n");
        header->valid = false;
        return;
    }
    header->restart_interval = marker_length(fileData);
}

void ReadHuffmanTable(file* fileData, ImageData* header) {
    printf("Reading huffman table...\n");
    int length = marker_length(fileData) - 2;
    byte table_info = get(fileData);
    while (length > 0) {
        byte index = table_info & 0x0F;
        if (index > 3) {
            printf("Invalid table ID: %d\n", (int)index);
            header->valid = false;
            return;
        }
        if (table_info >> 4)        //if AC table
            index += 4;

        if (header->huffman_tables[index].set) {
            printf("Multiple definitions of same huffman table: %d\n", (int)index);
            header->valid = false;
            return;
        }
        header->huffman_tables[index].set = true;
        header->huffman_tables[index].offset[0] = 0;
        for (int i = 1; i < 17; ++i)
            header->huffman_tables[index].offset[i] = header->huffman_tables[index].offset[i - 1] + get(fileData);
        if ((table_info >> 4) && header->huffman_tables[index].offset[16] > 162)
            printf("Too many symbols for AC huffman table: %d\n", (int)header->huffman_tables[index].offset[16]);
        if (!(table_info >> 4) && header->huffman_tables[index].offset[16] > 12)
            printf("Too many symbols for DC huffman table: %d\n", (int)header->huffman_tables[index].offset[16]);
        for (int i = 0; i < header->huffman_tables[index].offset[16]; ++i)
            header->huffman_tables[index].symbols[i] = get(fileData);

        length -= (1 + 16 + header->huffman_tables[index].offset[16]);
        //printf("%d\n", length);
    }

    if (length != 0) {
        printf("Invalid marker size for DHT marker.\n");
        header->valid = false;
    }
}

void ReadStartOfScan(file* fileData, ImageData* header) {
    printf("Reading SOS...\n");
    if (!header->SOFset) {
        printf("Bitstream started without SOF marker.\n");
        header->valid = false;
        return;
    }

    uint16_t length = marker_length(fileData) - 2;

    for (int i = 0; i < 3; ++i)
        header->color_components[i].set = false;

    byte count = get(fileData);
    if (count == 0 || count > 3) {
        printf("Invalid number of color components: %d\n", count);
        header->valid = false;
        return;
    }
    for (int i = 0; i < count; ++i) {
        byte id = get(fileData);
        bool start_with_zero = false;

        if (id == 0)
            start_with_zero = true;
        if (start_with_zero)
            id++;

        id--;
        if (header->color_components[id].set) {
            printf("Redefinition of color component id: %d\n", (int)id);
            header->valid = false;
            return;
        }
        header->color_components[id].set = true;

        byte huffman_table_id = get(fileData);
        byte dc_table_id = huffman_table_id >> 4;
        byte ac_table_id = huffman_table_id & 0x0F;
        if (dc_table_id > 3 || ac_table_id > 3) {
            printf("Invalid huffman table id\n");
            header->valid = false;
            return;
        }
        if (!header->huffman_tables[dc_table_id].set) {
            printf("This DC huffman table is not set, id: %d\n", dc_table_id);
            header->valid = false;
            return;
        }
        if (!header->huffman_tables[4 + ac_table_id].set) {
            printf("This AC huffman table is not set, id: %d\n", ac_table_id);
            header->valid = false;
            return;
        }
        header->color_components[id].DC_huffman_id = dc_table_id;
        header->color_components[id].AC_huffman_id = ac_table_id;
    }

    header->start_of_selection = get(fileData);
    header->end_of_selection = get(fileData);

    byte successive_approx = get(fileData);
    header->successive_approx_high = successive_approx >> 4;
    header->successive_approx_low = successive_approx & 0x0F;

    if (header->start_of_selection != 0 || header->end_of_selection != 63) {
        printf("Invalid spectral selection parameters\n");
        header->valid = false;
        return;
    }
    if (header->successive_approx_high != 0 || header->successive_approx_low != 0) {
        printf("Invalid successive approximation value");
        header->valid = false;
        return;
    }

    if (length - 4 - count * 2 != 0) {
        printf("Invalid marker length");
        header->valid = false;
        return;
    }
}

ImageData* ReadHeader(const char *fileName) {
    FILE* in_file;
    fopen_s(&in_file, fileName, "rb");
    if (!in_file) {
        printf("Cannot open file\n");
        return NULL;
    }

    file *fileData = (file*)malloc(sizeof(file));
    fseek(in_file, 0, SEEK_END);
    fileData->size = ftell(in_file);
    fileData->data = (byte*) malloc(fileData->size);
    fileData->pointer = 0;
    rewind(in_file);
    fread(fileData->data, sizeof(byte), fileData->size, in_file);
    fclose(in_file);

    ImageData* header = NewImage();
    if (!header) {
        printf("Cannot allocate space for Header\n");
        return NULL;
    }
    byte last = get(fileData);
    byte current = get(fileData);
    if (last != 0xFF || current != SOI) {
        printf("Not a jpeg file\n");
        return NULL;
    }

    last = get(fileData);
    current = get(fileData);
    while (header->valid) {
        if (fileData->pointer > fileData->size) {
            printf("File ended abruptly\n");
            header->valid = false;
            return header;
        }

        if (last != 0xFF) {
            printf("Not a marker: %X\n", (int)last);
            header->valid = false;
            return header;
        }

        else if (current >= APP0 && current <= APP15)
            ReadAPPN(fileData, header);
        else if (current == DQT)
            ReadQuantizationTable(fileData, header);
        else if (current == SOF0)
            ReadBaselineSOF(fileData, header);
        else if (current == DRI)
            ReadRestartInterval(fileData, header);
        else if (current == DHT)
            ReadHuffmanTable(fileData, header);
        else if (current == SOS) {
            ReadStartOfScan(fileData, header);
            break;
        }
        else if (current == COM)
            ReadComment(fileData, header);
        else if ((current >= JPG0 && current <= JPG13) || current == DHP || current == DNL || current == EXP)
            ReadComment(fileData, header);
        else if (current == TEM) {}
        else if (current == 0xFF) {
            current = get(fileData);
            continue;
        }

        last = get(fileData);
        current = get(fileData);
    }

    ReadBitStream(fileData, header);
    free(fileData->data);
    free(fileData);
    return header;
}

void PrintHeader(const ImageData* header) {
    for (int i = 0; i < 4; ++i) {
        if (!header->table_set[i])
            continue;
        printf("Quantization Table %d: \n", i);
        for (int j = 0; j < 64; ++j) {
            if (j > 0 && j % 8 == 0)
                printf("\n");
            printf("%d ", header->quant_table[i][j]);
        }
        printf("\n");
    }

    printf("---------------------------\n");

    printf("Frame type: 0x%X\n", (uint32_t)header->frametype);
    printf("Widht: %d\n", header->width);
    printf("Height: %d\n", header->height);
    printf("Mcu width: %d\n", header->mcu_width);
    printf("Mcu height: %d\n", header->mcu_height);
    printf("Mcu width padded: %d\n", header->mcu_width_padded);
    printf("Mcu height padded: %d\n", header->mcu_height_padded);

    printf("--------------------------\n");

    printf("Restart Interval: %d\n", header->restart_interval);
    printf("--------------------------\n");

    printf("max x sampling factor: %d\n", header->Xsampling_factor);
    printf("max y sampling factor: %d\n", header->Ysampling_factor);
    if (header->SOFset) {
        for (int i = 0; i < 3; ++i) {
            if (!header->color_components[i].set)
                continue;
            printf("Color Component ID: %d\n", i + 1);
            printf("X sampling factor: %d\n", (uint16_t)header->color_components[i].Xsampling_factor);
            printf("Y sampling factor: %d\n", (uint16_t)header->color_components[i].Ysampling_factor);
            printf("Qunatization table ID: %d\n", (uint16_t)header->color_components[i].quant_table_id);
            printf("AC huffman table ID: %d\n", (uint16_t)header->color_components[i].AC_huffman_id);
            printf("DC huffman table ID: %d\n", (uint16_t)header->color_components[i].DC_huffman_id);
            printf("\n");
        }
    }

    printf("--------------------------\n");
    printf("Huffman Tables:\n");
    for (int i = 0; i < 8; ++i) {
        if (header->huffman_tables[i].set) {
            if (i < 4)
                printf("DC table %d:\n", i);
            else
                printf("AC table %d:\n",i - 4);
            for (int j = 0; j < 16; ++j) {
                if (header->huffman_tables[i].offset[j + 1] - header->huffman_tables[i].offset[j] > 0)
                    printf("%d bit:", j + 1);
                for (int k = header->huffman_tables[i].offset[j]; k < header->huffman_tables[i].offset[j + 1]; ++k) {
                    printf("%X("  BYTE_TO_BINARY_PATTERN  BYTE_TO_BINARY_PATTERN") ", (uint16_t)header->huffman_tables[i].symbols[k], BYTE_TO_BINARY(header->huffman_tables[i].codes[k] >> 8), BYTE_TO_BINARY(header->huffman_tables[i].codes[k] & 0x00FF));
                }
                if (header->huffman_tables[i].offset[j + 1] - header->huffman_tables[i].offset[j] > 0)
                    printf("\n");
            }
        }
    }

    printf("--------------------------\n");
    printf("SOS Marker:\n");
    printf("Start of Selection: %d\n", (int)header->start_of_selection);
    printf("End of Selection: %d\n", (int)header->end_of_selection);
    printf("Successive approximation high: %d\n", (int)header->successive_approx_high);
    printf("Successive approximation low: %d\n", (int)header->successive_approx_low);
    printf("--------------------------\n");
    printf("Size of bitstream: %d\n", header->bitsream->size);
}

float m0;
float m1;
float m3;
float m5;
float m2;
float m4;

float s0;
float s1;
float s2;
float s3;
float s4;
float s5;
float s6;
float s7;

void InitConstants() {
    m0 = 2.0 * cos(1.0 / 16.0 * 2.0 * PI);
    m1 = 2.0 * cos(2.0 / 16.0 * 2.0 * PI);
    m3 = 2.0 * cos(2.0 / 16.0 * 2.0 * PI);
    m5 = 2.0 * cos(3.0 / 16.0 * 2.0 * PI);
    m2 = m0 - m5;
    m4 = m0 + m5;

    s0 = cos(0.0 / 16.0 * PI) / sqrt(8);
    s1 = cos(1.0 / 16.0 * PI) / 2.0;
    s2 = cos(2.0 / 16.0 * PI) / 2.0;
    s3 = cos(3.0 / 16.0 * PI) / 2.0;
    s4 = cos(4.0 / 16.0 * PI) / 2.0;
    s5 = cos(5.0 / 16.0 * PI) / 2.0;
    s6 = cos(6.0 / 16.0 * PI) / 2.0;
    s7 = cos(7.0 / 16.0 * PI) / 2.0;
}


