#include "include/jpg.h"
#include "include/bmp.h"
#include <math.h>
#include <stdint.h>

#include "../libs/SDL2/SDL2-2.0.14/SDL2/SDL.h"

extern const uint8_t indices[];
extern float m0;
extern float m1;
extern float m3;
extern float m5;
extern float m2;
extern float m4;

extern float s0;
extern float s1;
extern float s2;
extern float s3;
extern float s4;
extern float s5;
extern float s6;
extern float s7;

bool DequantizeComponent(int* comp, uint16_t* qTable) {
    for (int i = 0; i < 64; ++i) {
        comp[i] *= qTable[i];
    }
    return true;
}

bool Dequantize(MCU* mcus, const ImageData* header) {
    for (int y = 0; y < header->mcu_height; y += header->Ysampling_factor) {
        for(int x = 0; x < header->mcu_width; x += header->Xsampling_factor) {
            for (int j = 0; j < header->num_componenets; ++j) {
                for(int v = 0; v < header->color_components[j].Ysampling_factor; ++v){
                    for(int h = 0; h < header->color_components[j].Xsampling_factor; ++h){
                        if(!DequantizeComponent(COMPONENT(mcus[(y + v) * header->mcu_width_padded + (x + h)], j), header->quant_table[header->color_components[j].quant_table_id]))
                            return false;
                    }
                }
            }
        }
    }
    return true;
}

void InverseDCTComponent(int* component) {
    for (uint8_t i = 0; i < 8; ++i) {
        const float g0 = component[0 * 8 + i] * s0;
        const float g1 = component[4 * 8 + i] * s4;
        const float g2 = component[2 * 8 + i] * s2;
        const float g3 = component[6 * 8 + i] * s6;
        const float g4 = component[5 * 8 + i] * s5;
        const float g5 = component[1 * 8 + i] * s1;
        const float g6 = component[7 * 8 + i] * s7;
        const float g7 = component[3 * 8 + i] * s3;

        const float f0 = g0;
        const float f1 = g1;
        const float f2 = g2;
        const float f3 = g3;
        const float f4 = g4 - g7;
        const float f5 = g5 + g6;
        const float f6 = g5 - g6;
        const float f7 = g4 + g7;

        const float e0 = f0;
        const float e1 = f1;
        const float e2 = f2 - f3;
        const float e3 = f2 + f3;
        const float e4 = f4;
        const float e5 = f5 - f7;
        const float e6 = f6;
        const float e7 = f5 + f7;
        const float e8 = f4 + f6;

        const float d0 = e0;
        const float d1 = e1;
        const float d2 = e2 * m1;
        const float d3 = e3;
        const float d4 = e4 * m2;
        const float d5 = e5 * m3;
        const float d6 = e6 * m4;
        const float d7 = e7;
        const float d8 = e8 * m5;

        const float c0 = d0 + d1;
        const float c1 = d0 - d1;
        const float c2 = d2 - d3;
        const float c3 = d3;
        const float c4 = d4 + d8;
        const float c5 = d5 + d7;
        const float c6 = d6 - d8;
        const float c7 = d7;
        const float c8 = c5 - c6;

        const float b0 = c0 + c3;
        const float b1 = c1 + c2;
        const float b2 = c1 - c2;
        const float b3 = c0 - c3;
        const float b4 = c4 - c8;
        const float b5 = c8;
        const float b6 = c6 - c7;
        const float b7 = c7;

        component[0 * 8 + i] = b0 + b7;
        component[1 * 8 + i] = b1 + b6;
        component[2 * 8 + i] = b2 + b5;
        component[3 * 8 + i] = b3 + b4;
        component[4 * 8 + i] = b3 - b4;
        component[5 * 8 + i] = b2 - b5;
        component[6 * 8 + i] = b1 - b6;
        component[7 * 8 + i] = b0 - b7;
    }
    for (uint8_t i = 0; i < 8; ++i) {
        const float g0 = component[i * 8 + 0] * s0;
        const float g1 = component[i * 8 + 4] * s4;
        const float g2 = component[i * 8 + 2] * s2;
        const float g3 = component[i * 8 + 6] * s6;
        const float g4 = component[i * 8 + 5] * s5;
        const float g5 = component[i * 8 + 1] * s1;
        const float g6 = component[i * 8 + 7] * s7;
        const float g7 = component[i * 8 + 3] * s3;

        const float f0 = g0;
        const float f1 = g1;
        const float f2 = g2;
        const float f3 = g3;
        const float f4 = g4 - g7;
        const float f5 = g5 + g6;
        const float f6 = g5 - g6;
        const float f7 = g4 + g7;

        const float e0 = f0;
        const float e1 = f1;
        const float e2 = f2 - f3;
        const float e3 = f2 + f3;
        const float e4 = f4;
        const float e5 = f5 - f7;
        const float e6 = f6;
        const float e7 = f5 + f7;
        const float e8 = f4 + f6;

        const float d0 = e0;
        const float d1 = e1;
        const float d2 = e2 * m1;
        const float d3 = e3;
        const float d4 = e4 * m2;
        const float d5 = e5 * m3;
        const float d6 = e6 * m4;
        const float d7 = e7;
        const float d8 = e8 * m5;

        const float c0 = d0 + d1;
        const float c1 = d0 - d1;
        const float c2 = d2 - d3;
        const float c3 = d3;
        const float c4 = d4 + d8;
        const float c5 = d5 + d7;
        const float c6 = d6 - d8;
        const float c7 = d7;
        const float c8 = c5 - c6;

        const float b0 = c0 + c3;
        const float b1 = c1 + c2;
        const float b2 = c1 - c2;
        const float b3 = c0 - c3;
        const float b4 = c4 - c8;
        const float b5 = c8;
        const float b6 = c6 - c7;
        const float b7 = c7;

        component[i * 8 + 0] = b0 + b7;
        component[i * 8 + 1] = b1 + b6;
        component[i * 8 + 2] = b2 + b5;
        component[i * 8 + 3] = b3 + b4;
        component[i * 8 + 4] = b3 - b4;
        component[i * 8 + 5] = b2 - b5;
        component[i * 8 + 6] = b1 - b6;
        component[i * 8 + 7] = b0 - b7;
    }
}

void InverseDCT(const ImageData* header, MCU* mcus) {
    for (int y = 0; y < header->mcu_height; y += header->Ysampling_factor) {
        for(int x = 0; x < header->mcu_width; x += header->Xsampling_factor){
            for(int j = 0; j < header->num_componenets; ++j){
                for(int v = 0; v < header->color_components[j].Ysampling_factor; ++v){
                    for(int h = 0; h < header->color_components[j].Xsampling_factor; ++h){
                        InverseDCTComponent(COMPONENT(mcus[(y + v) * header->mcu_width_padded + (x + h)], j));
                    }
                }
            }
        }
    }
}

void ColorConversion(ImageData *header, MCU *mcu, const MCU *cbcr, int v, int h) {
    for (uint8_t y = 7; y < 8; --y) {
        for (uint8_t x = 7; x < 8; --x) {
            const uint8_t pixel = y * 8 + x;
            const uint8_t cbcrPixelRow = y / header->Ysampling_factor + 4 * v;
            const uint8_t cbcrPixelColumn = x / header->Xsampling_factor + 4 * h;
            const uint8_t cbcrPixel = cbcrPixelRow * 8 + cbcrPixelColumn;
            int r = mcu->y[pixel]                                    + 1.402f * cbcr->cr[cbcrPixel] + 128;
            int g = mcu->y[pixel] - 0.344f * cbcr->cb[cbcrPixel] - 0.714f * cbcr->cr[cbcrPixel] + 128;
            int b = mcu->y[pixel] + 1.772f * cbcr->cb[cbcrPixel]                                    + 128;
            if (r < 0)   r = 0;
            if (r > 255) r = 255;
            if (g < 0)   g = 0;
            if (g > 255) g = 255;
            if (b < 0)   b = 0;
            if (b > 255) b = 255;
            mcu->r[pixel] = r;
            mcu->g[pixel] = g;
            mcu->b[pixel] = b;
        }
    }
}

void YCbCrtoRGB(ImageData* header, MCU* mcus) {
    for (int y = 0; y < header->mcu_height; y += header->Ysampling_factor) {
        for(int x = 0; x < header->mcu_width; x += header->Xsampling_factor) {
            const MCU *cbcr = &mcus[y * header->mcu_width_padded + x];
            for(uint8_t v = header->Ysampling_factor - 1; v < header->Ysampling_factor; --v) {
                for(uint8_t h = header->Xsampling_factor - 1; h < header->Xsampling_factor; --h) {
                    MCU* mcu = &mcus[(y + v) * header->mcu_width_padded + (x + h)];
                    ColorConversion(header, mcu, cbcr, v, h);
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("No input files provided\n");
        return 1;
    }

    ImageData* header = ReadHeader(argv[1]);
    if(!header->valid) {
        printf("Invalid header\n");
        return 1;
    }

    GenerateHuffmanCodes(header);
    //PrintHeader(header);
    MCU* mcus = HuffmanDecoder(header);
    free(header->bitsream->data);
    free(header->bitsream);

    Dequantize(mcus, header);
    for(int i = 0; i < 4; ++i){
        if(header->table_set[i])
            free(header->quant_table[i]);
    } 

    InitConstants();
    InverseDCT(header, mcus);

    YCbCrtoRGB(header, mcus);

    Image img = CreateImage(header->width, header->height);
    for (uint16_t j = 0; j < img.height; ++j) {
        const int blockRow = j / 8; 
        const int pixelRow = j % 8;
        for (int i = 0; i < img.width; ++i) {
            const int blockColumn = i / 8;
            const int pixelColumn = i % 8;
            img.pixel_data[j * img.width + i] = rgb_to_hex(mcus[blockRow * header->mcu_width_padded + blockColumn].r[pixelRow * 8 + pixelColumn],
                                                            mcus[blockRow * header->mcu_width_padded + blockColumn].g[pixelRow * 8 + pixelColumn],
                                                            mcus[blockRow * header->mcu_width_padded + blockColumn].b[pixelRow * 8 + pixelColumn]);
        }
    }
    //SaveImage(&img, "hello.bmp");
    free(mcus);
    free(header);

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("Failed to initialize SDL\n");
        return 1;
    }

    Uint32 rmask, gmask, bmask, amask;
    amask = 0x00000000;
    rmask = 0x00ff0000;
    gmask = 0x0000ff00;
    bmask = 0x000000ff;

    SDL_Window *window = SDL_CreateWindow(argv[1], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, img.width, img.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(img.pixel_data, img.width, img.height, 32, 4 * img.width, rmask, gmask, bmask, amask);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetWindowMinimumSize(window, img.width, img.height);
    free(img.pixel_data);

    if(window == NULL || renderer == NULL || texture == NULL) {
        printf("Failure\n");
        return 1;
    }
    SDL_FreeSurface(surface);

    uint32_t **pixels = (uint32_t**)&surface->pixels;
    *pixels = img.pixel_data;   

    bool is_running = true;
    SDL_Event e;
    while(is_running) {
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT)
                is_running = false;
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        SDL_Rect dest = {(w - img.width) / 2, (h - img.height) / 2, img.width, img.height};
        SDL_RenderCopy(renderer, texture, NULL, &dest);
        SDL_RenderPresent(renderer);

        SDL_Delay(50);
    } 

    return 0;
}