/**
 * @file texture.h
 * @brief Static Memory Pool Texture System - NO MALLOC
 */

#ifndef TEXTURE_H
#define TEXTURE_H
#include "engine_config.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* Pool configuration */
#define MAX_TEXTURE_PIXELS      (256 * 256 * 4)  /* ~256KB for all textures */

#ifdef SDL_PC
    extern uint16_t g_pixel_pool[MAX_TEXTURE_PIXELS];
#else
    extern uint16_t g_pixel_pool[MAX_TEXTURE_PIXELS] SECTION_SDRAM;
#endif

/* Texture descriptor */
typedef struct {
    uint32_t pixel_start;       /* Offset into pixel pool */
    uint16_t width;
    uint16_t height;
    uint16_t width_mask;        /* width - 1 for power-of-2 */
    uint16_t height_mask;
    uint8_t in_use;
    uint8_t flags;
} TextureSlot_t;

/* RGB565 helpers */
#define RGB565(r,g,b) ((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|((b)>>3))

#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_GRAY      0x8410
#define COLOR_DARK_GRAY 0x4208

/* API */
void Texture_Init(void);

uint32_t Texture_LoadBMP(const void* data, uint32_t size);
uint32_t Texture_CreateSolid(uint16_t color, uint16_t w, uint16_t h);
uint32_t Texture_CreateCheckerboard(uint16_t c1, uint16_t c2, uint16_t size);

TextureSlot_t* Texture_Get(uint32_t id);
uint16_t* Texture_GetPixels(uint32_t id);

uint16_t Texture_SampleFast(uint32_t id, int u, int v);

void Texture_Free(uint32_t id);
uint32_t Texture_GetFreePixels(void);

#ifdef __cplusplus
}
#endif

#endif
