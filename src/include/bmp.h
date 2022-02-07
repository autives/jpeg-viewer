
#define BMP_IMPLEMENTATION
#ifndef BMP
#define BMP

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t type;
	uint32_t file_size;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;
	uint32_t info_size;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bitcount;
	uint32_t compression;
	uint32_t size_image;
	uint32_t x_pels_per_meter;
	uint32_t y_pels_per_meter;
	uint32_t clr_used;
	uint32_t clr_important;
} BMPHeader;
#pragma pack(pop)

typedef struct {
    BMPHeader header;
    uint32_t width;
    uint32_t height;
    uint32_t* pixel_data;
} Image;

Image CreateImage(uint32_t width, uint32_t height);
void SaveImage(Image * img, const char * file);


#ifdef BMP_IMPLEMENTATION
Image CreateImage(uint32_t width, uint32_t height){
    Image img;
    img.width = width;
    img.height = height;

    img.header.type = 0x4D42;
    img.header.file_size = sizeof(BMPHeader) + width * height * sizeof(uint32_t);
    img.header.reserved1 = 0;
    img.header.reserved2 = 0;
    img.header.offset = sizeof(BMPHeader);
    img.header.info_size = sizeof(BMPHeader) - 14;
    img.header.width = width;
    img.header.height = -(int)height;
    img.header.planes = 1;
    img.header.bitcount = 32;
    img.header.compression = 0;
    img.header.size_image = 0;
    img.header.x_pels_per_meter = 0;
    img.header.y_pels_per_meter = 0;
    img.header.clr_used = 0;
    img.header.clr_important = 0;

    img.pixel_data = (uint32_t*)malloc(width * height * sizeof(uint32_t));

    return img;
}

void SaveImage(Image * img, const char * file){
    FILE * dest;
    fopen_s(&dest, file, "wb");
    fwrite(&img->header, sizeof(BMPHeader), 1, dest);
    fwrite(img->pixel_data, sizeof(uint32_t), img->width * img->height, dest);
    fclose(dest);
}

uint32_t rgb_to_hex(uint8_t r, uint8_t g, uint8_t b){
    return r << 16 | g << 8 | b;
}
#endif
#endif