/**
 * @file texture.cpp
 * @brief Static Memory Pool Texture System
 */

#include "texture.h"
#include "platform.h"
#include <string.h>
#include "engine_config.h"

 /* ============================================================
  * Static Memory Pools
  * ============================================================ */

#ifdef SDL_PC
uint16_t g_pixel_pool[MAX_TEXTURE_PIXELS];
#else
uint16_t g_pixel_pool[MAX_TEXTURE_PIXELS] SECTION_SDRAM;
#endif

static uint32_t g_pixel_used = 0;
static TextureSlot_t g_textures[MAX_TEXTURES];

/* ============================================================
 * Initialization
 * ============================================================ */

void Texture_Init(void)
{
    memset(g_textures, 0, sizeof(g_textures));
    g_pixel_used = 0;
}

/* ============================================================
 * Allocation
 * ============================================================ */

static uint32_t AllocTextureSlot(void)
{
    for (uint32_t i = 0; i < MAX_TEXTURES; i++) {
        if (!g_textures[i].in_use) return i;
    }
    return 0xFFFFFFFF;
}

static uint32_t AllocPixels(uint32_t count)
{
    if (g_pixel_used + count > MAX_TEXTURE_PIXELS) return 0xFFFFFFFF;
    uint32_t start = g_pixel_used;
    g_pixel_used += count;
    return start;
}

/* ============================================================
 * Texture Creation
 * ============================================================ */

uint32_t Texture_CreateSolid(uint16_t color, uint16_t w, uint16_t h)
{
    uint32_t slot = AllocTextureSlot();
    if (slot == 0xFFFFFFFF) return 0xFFFFFFFF;

    uint32_t pixel_count = w * h;
    uint32_t pixel_start = AllocPixels(pixel_count);
    if (pixel_start == 0xFFFFFFFF) return 0xFFFFFFFF;

    uint16_t* pixels = &g_pixel_pool[pixel_start];
    for (uint32_t i = 0; i < pixel_count; i++) {
        pixels[i] = color;
    }

    g_textures[slot].pixel_start = pixel_start;
    g_textures[slot].width = w;
    g_textures[slot].height = h;
    g_textures[slot].width_mask = w - 1;
    g_textures[slot].height_mask = h - 1;
    g_textures[slot].in_use = 1;
    g_textures[slot].flags = 0;

    return slot;
}

uint32_t Texture_CreateCheckerboard(uint16_t c1, uint16_t c2, uint16_t size)
{
    uint32_t slot = AllocTextureSlot();
    if (slot == 0xFFFFFFFF) return 0xFFFFFFFF;

    uint32_t pixel_count = size * size;
    uint32_t pixel_start = AllocPixels(pixel_count);
    if (pixel_start == 0xFFFFFFFF) return 0xFFFFFFFF;

    uint16_t* pixels = &g_pixel_pool[pixel_start];
    int check = size / 8;
    if (check < 1) check = 1;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int cx = x / check, cy = y / check;
            pixels[y * size + x] = ((cx + cy) & 1) ? c1 : c2;
        }
    }

    g_textures[slot].pixel_start = pixel_start;
    g_textures[slot].width = size;
    g_textures[slot].height = size;
    g_textures[slot].width_mask = size - 1;
    g_textures[slot].height_mask = size - 1;
    g_textures[slot].in_use = 1;
    g_textures[slot].flags = 0;

    return slot;
}

/* ============================================================
 * Accessors
 * ============================================================ */

TextureSlot_t* Texture_Get(uint32_t id)
{
    if (id >= MAX_TEXTURES || !g_textures[id].in_use) return NULL;
    return &g_textures[id];
}

uint16_t* Texture_GetPixels(uint32_t id)
{
    if (id >= MAX_TEXTURES || !g_textures[id].in_use) return NULL;
    return &g_pixel_pool[g_textures[id].pixel_start];
}



uint16_t Texture_SampleFast(uint32_t id, int u, int v)
{
    if (id >= MAX_TEXTURES || !g_textures[id].in_use) return 0xF81F;

    TextureSlot_t* tex = &g_textures[id];
    int tx = u & tex->width_mask;
    int ty = v & tex->height_mask;

    return g_pixel_pool[tex->pixel_start + ty * tex->width + tx];
}

/* ============================================================
 * Cleanup
 * ============================================================ */

void Texture_Free(uint32_t id)
{
    if (id < MAX_TEXTURES) {
        g_textures[id].in_use = 0;
    }
}

uint32_t Texture_GetFreePixels(void)
{
    return MAX_TEXTURE_PIXELS - g_pixel_used;
}