/**
 * @file loader_bmp.cpp
 * @brief BMP Texture Loader
 */

#include "texture.h"
#include "platform.h"
#include <string.h>
#include <stdlib.h>

 /* BMP File Header (14 bytes) */
#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPFileHeader_t;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_ppm;
    int32_t y_ppm;
    uint32_t colors_used;
    uint32_t colors_important;
} BMPInfoHeader_t;
#pragma pack(pop)

static inline uint16_t RGB_to_565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static void ConvertRow_BGR24(uint16_t* dst, const uint8_t* src, int width)
{
    for (int x = 0; x < width; x++) {
        uint8_t b = src[x * 3 + 0];
        uint8_t g = src[x * 3 + 1];
        uint8_t r = src[x * 3 + 2];
        dst[x] = RGB_to_565(r, g, b);
    }
}

static void ConvertRow_BGRA32(uint16_t* dst, const uint8_t* src, int width)
{
    for (int x = 0; x < width; x++) {
        uint8_t b = src[x * 4 + 0];
        uint8_t g = src[x * 4 + 1];
        uint8_t r = src[x * 4 + 2];
        dst[x] = RGB_to_565(r, g, b);
    }
}

static void ConvertRow_Indexed8(uint16_t* dst, const uint8_t* src, int width,
    const uint8_t* palette)
{
    for (int x = 0; x < width; x++) {
        int idx = src[x] * 4;
        uint8_t b = palette[idx + 0];
        uint8_t g = palette[idx + 1];
        uint8_t r = palette[idx + 2];
        dst[x] = RGB_to_565(r, g, b);
    }
}

uint32_t Texture_LoadBMP_Memory(const void* data, uint32_t size)
{
    if (!data || size < sizeof(BMPFileHeader_t) + sizeof(BMPInfoHeader_t)) {
        return 0xFFFFFFFF;
    }

    const uint8_t* ptr = (const uint8_t*)data;
    const BMPFileHeader_t* file_header = (const BMPFileHeader_t*)ptr;

    if (file_header->type != 0x4D42) {
        return 0xFFFFFFFF;
    }

    const BMPInfoHeader_t* info = (const BMPInfoHeader_t*)(ptr + sizeof(BMPFileHeader_t));

    int width = info->width;
    int height = info->height;
    int top_down = 0;

    if (height < 0) {
        height = -height;
        top_down = 1;
    }

    if (width <= 0 || height <= 0 || width > 1024 || height > 1024) {
        return 0xFFFFFFFF;
    }

    /* Use pool-based texture allocation */
    uint32_t tex_id;
    if (info->bpp == 24 || info->bpp == 32 || info->bpp == 8) {
        tex_id = Texture_CreateSolid(0x0000, (uint16_t)width, (uint16_t)height);
    }
    else {
        return 0xFFFFFFFF;
    }

    if (tex_id == 0xFFFFFFFF) {
        return 0xFFFFFFFF;
    }

    uint16_t* pixels = Texture_GetPixels(tex_id);
    if (!pixels) {
        Texture_Free(tex_id);
        return 0xFFFFFFFF;
    }

    const uint8_t* palette = NULL;
    if (info->bpp <= 8) {
        palette = ptr + sizeof(BMPFileHeader_t) + info->size;
    }

    const uint8_t* pixel_data = ptr + file_header->offset;

    int row_size;
    switch (info->bpp) {
    case 8:  row_size = (width + 3) & ~3; break;
    case 24: row_size = ((width * 3) + 3) & ~3; break;
    case 32: row_size = width * 4; break;
    default:
        Texture_Free(tex_id);
        return 0xFFFFFFFF;
    }

    for (int y = 0; y < height; y++) {
        int src_y = top_down ? y : (height - 1 - y);
        const uint8_t* src_row = pixel_data + src_y * row_size;
        uint16_t* dst_row = pixels + y * width;

        switch (info->bpp) {
        case 8:
            ConvertRow_Indexed8(dst_row, src_row, width, palette);
            break;
        case 24:
            ConvertRow_BGR24(dst_row, src_row, width);
            break;
        case 32:
            ConvertRow_BGRA32(dst_row, src_row, width);
            break;
        }
    }

    return tex_id;
}

uint32_t Texture_LoadBMP(const void* data, uint32_t size)
{
    return Texture_LoadBMP_Memory(data, size);
}