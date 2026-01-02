/**
 * @file rasterizer.cpp
 * @brief Software Triangle Rasterizer Implementation
 */

#include "rasterizer.h"
#include "engine_config.h"
#include <string.h>
#include <stdint.h>

#ifdef SDL_PC
#include "device.h"
static Device* g_device = NULL;
#else
#include "display.h"
 /* Z-buffer in DTCM for fast access */
DTCM_BSS static uint16_t zbuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT];
static uint16_t* g_framebuffer = NULL;
#endif

static RasterizerStats_t g_stats;

/* Edge function: positive if point is on left side of edge */
static inline int32_t EdgeFunction(int32_t v0x, int32_t v0y,
    int32_t v1x, int32_t v1y,
    int32_t px, int32_t py)
{
    return (v1x - v0x) * (py - v0y) - (v1y - v0y) * (px - v0x);
}

void Rasterizer_Init(void)
{
#ifdef SDL_PC
    g_device = NULL;
#else
    g_framebuffer = NULL;
#endif
    memset(&g_stats, 0, sizeof(g_stats));
}

#ifdef SDL_PC
void Rasterizer_SetDevice(Device* device)
{
    g_device = device;
}
#else
void Rasterizer_SetFrameBuffer(uint16_t* fb)
{
    g_framebuffer = fb;
}
#endif

void Rasterizer_Clear(uint16_t color)
{
#ifdef SDL_PC
    if (!g_device) return;

    /* Convert RGB565 to Color */
    uint8_t r = ((color >> 11) & 0x1F) << 3;
    uint8_t g = ((color >> 5) & 0x3F) << 2;
    uint8_t b = (color & 0x1F) << 3;
    g_device->Clear(Color(r, g, b));
#else
    if (!g_framebuffer) return;

    uint32_t color32 = ((uint32_t)color << 16) | color;
    uint32_t* fb32 = (uint32_t*)g_framebuffer;
    int count = (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 2;

    for (int i = 0; i < count; i++) {
        fb32[i] = color32;
    }

    Rasterizer_ClearDepth();
#endif
    Rasterizer_ResetStats();
}

void Rasterizer_ClearDepth(void)
{
#ifdef SDL_PC
    if (g_device) {
        g_device->ClearDepth();  // Actually clear it!
    }
#else
    uint32_t* zb32 = (uint32_t*)zbuffer;
    int count = (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 2;

    for (int i = 0; i < count; i++) {
        zb32[i] = 0xFFFFFFFF;
    }
#endif
}

uint16_t Texture_Sample(const Texture_t* tex, float u, float v)
{
    u = u - (int)u; if (u < 0) u += 1.0f;
    v = v - (int)v; if (v < 0) v += 1.0f;

    int tx = (int)(u * tex->width) & tex->width_mask;
    int ty = (int)(v * tex->height) & tex->height_mask;

    return tex->pixels[ty * tex->width + tx];
}

static inline uint16_t ColorLerp(uint16_t c0, uint16_t c1, uint16_t c2,
    float b0, float b1, float b2)
{
    int r = (int)(b0 * ((c0 >> 11) & 0x1F) + b1 * ((c1 >> 11) & 0x1F) + b2 * ((c2 >> 11) & 0x1F) + 0.5f);
    int g = (int)(b0 * ((c0 >> 5) & 0x3F) + b1 * ((c1 >> 5) & 0x3F) + b2 * ((c2 >> 5) & 0x3F) + 0.5f);
    int b = (int)(b0 * (c0 & 0x1F) + b1 * (c1 & 0x1F) + b2 * (c2 & 0x1F) + 0.5f);

    if (r > 31) r = 31; if (g > 63) g = 63; if (b > 31) b = 31;
    return (uint16_t)((r << 11) | (g << 5) | b);
}

static inline uint16_t ColorModulate(uint16_t texel, uint16_t light)
{
    int tr = (texel >> 11) & 0x1F, tg = (texel >> 5) & 0x3F, tb = texel & 0x1F;
    int lr = (light >> 11) & 0x1F, lg = (light >> 5) & 0x3F, lb = light & 0x1F;
    return (uint16_t)(((tr * lr) >> 5 << 11) | ((tg * lg) >> 6 << 5) | ((tb * lb) >> 5));
}

/* Helper: Convert RGB565 to Color (PC only) */
#ifdef SDL_PC
static inline Color RGB565ToColor(uint16_t c)
{
    uint8_t r = ((c >> 11) & 0x1F) << 3;
    uint8_t g = ((c >> 5) & 0x3F) << 2;
    uint8_t b = (c & 0x1F) << 3;
    return Color(r, g, b);
}
#endif

void Rasterizer_DrawTriangle(const ScreenVertex_t* v0, const ScreenVertex_t* v1,
    const ScreenVertex_t* v2, const Texture_t* texture)
{
#ifdef SDL_PC
    if (!g_device) return;
    int width = g_device->Width();
    int height = g_device->Height();
#else
    if (!g_framebuffer) return;
    int width = DISPLAY_WIDTH;
    int height = DISPLAY_HEIGHT;
#endif

    g_stats.triangles_submitted++;

    int x0 = v0->x, y0 = v0->y;
    int x1 = v1->x, y1 = v1->y;
    int x2 = v2->x, y2 = v2->y;

    int32_t area = EdgeFunction(x0, y0, x1, y1, x2, y2);
    if (area <= 0) { g_stats.triangles_culled++; return; }

    int minX = Clampi(Min3i(x0, x1, x2), 0, width - 1);
    int maxX = Clampi(Max3i(x0, x1, x2), 0, width - 1);
    int minY = Clampi(Min3i(y0, y1, y2), 0, height - 1);
    int maxY = Clampi(Max3i(y0, y1, y2), 0, height - 1);

    if (minX > maxX || minY > maxY) { g_stats.triangles_culled++; return; }

    float invArea = 1.0f / (float)area;

    int32_t A12 = y1 - y2, B12 = x2 - x1;
    int32_t A20 = y2 - y0, B20 = x0 - x2;
    int32_t A01 = y0 - y1, B01 = x1 - x0;

    int32_t w0_row = EdgeFunction(x1, y1, x2, y2, minX, minY);
    int32_t w1_row = EdgeFunction(x2, y2, x0, y0, minX, minY);
    int32_t w2_row = EdgeFunction(x0, y0, x1, y1, minX, minY);

    float w0_inv = v0->w_inv, w1_inv = v1->w_inv, w2_inv = v2->w_inv;

    for (int y = minY; y <= maxY; y++) {
        int32_t w0 = w0_row, w1 = w1_row, w2 = w2_row;

        for (int x = minX; x <= maxX; x++) {
            if ((w0 | w1 | w2) >= 0) {
                float b0 = w0 * invArea, b1 = w1 * invArea, b2 = w2 * invArea;
                float z = b0 * v0->z + b1 * v1->z + b2 * v2->z;

                uint16_t color565;
                if (texture) {
                    float w = b0 * w0_inv + b1 * w1_inv + b2 * w2_inv;
                    float inv_w = 1.0f / w;
                    float u = (b0 * v0->u * w0_inv + b1 * v1->u * w1_inv + b2 * v2->u * w2_inv) * inv_w;
                    float v = (b0 * v0->v * w0_inv + b1 * v1->v * w1_inv + b2 * v2->v * w2_inv) * inv_w;
                    uint16_t texel = Texture_Sample(texture, u, v);
                    uint16_t light = ColorLerp(v0->color, v1->color, v2->color, b0, b1, b2);
                    color565 = ColorModulate(texel, light);
                }
                else {
                    color565 = ColorLerp(v0->color, v1->color, v2->color, b0, b1, b2);
                }

#ifdef SDL_PC
                /* Use Device::PutPixel with depth test */
                g_device->PutPixel(x, y, z, RGB565ToColor(color565));
                g_stats.pixels_drawn++;
#else
                /* Direct framebuffer access with manual depth test */
                uint16_t z16 = (uint16_t)(z);
                int idx = y * DISPLAY_WIDTH + x;
                if (z16 < zbuffer[idx]) {
                    zbuffer[idx] = z16;
                    g_framebuffer[idx] = color565;
                    g_stats.pixels_drawn++;
                }
#endif
            }
            w0 += A12; w1 += A20; w2 += A01;
        }
        w0_row += B12; w1_row += B20; w2_row += B01;
    }
    g_stats.triangles_drawn++;
}

void Rasterizer_DrawTriangleSolid(const ScreenVertex_t* v0, const ScreenVertex_t* v1,
    const ScreenVertex_t* v2, uint16_t color)
{
#ifdef SDL_PC
    if (!g_device) return;
    int width = g_device->Width();
    int height = g_device->Height();
    Color col = RGB565ToColor(color);
#else
    if (!g_framebuffer) return;
    int width = DISPLAY_WIDTH;
    int height = DISPLAY_HEIGHT;
#endif

    g_stats.triangles_submitted++;

    int x0 = v0->x, y0 = v0->y;
    int x1 = v1->x, y1 = v1->y;
    int x2 = v2->x, y2 = v2->y;

    int32_t area = EdgeFunction(x0, y0, x1, y1, x2, y2);
    if (area <= 0) { g_stats.triangles_culled++; return; }

    int minX = Clampi(Min3i(x0, x1, x2), 0, width - 1);
    int maxX = Clampi(Max3i(x0, x1, x2), 0, width - 1);
    int minY = Clampi(Min3i(y0, y1, y2), 0, height - 1);
    int maxY = Clampi(Max3i(y0, y1, y2), 0, height - 1);

    if (minX > maxX || minY > maxY) { g_stats.triangles_culled++; return; }

    float invArea = 1.0f / (float)area;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            int32_t w0 = EdgeFunction(x1, y1, x2, y2, x, y);
            int32_t w1 = EdgeFunction(x2, y2, x0, y0, x, y);
            int32_t w2 = EdgeFunction(x0, y0, x1, y1, x, y);

            if ((w0 | w1 | w2) >= 0) {
                float b0 = w0 * invArea, b1 = w1 * invArea, b2 = w2 * invArea;
                float z = b0 * v0->z + b1 * v1->z + b2 * v2->z;

#ifdef SDL_PC
                g_device->PutPixel(x, y, z, col);
                g_stats.pixels_drawn++;
#else
                uint16_t z16 = (uint16_t)(z);
                int idx = y * DISPLAY_WIDTH + x;
                if (z16 < zbuffer[idx]) {
                    zbuffer[idx] = z16;
                    g_framebuffer[idx] = color;
                    g_stats.pixels_drawn++;
                }
#endif
            }
        }
    }
    g_stats.triangles_drawn++;
}

void Rasterizer_DrawLine(int x0, int y0, int x1, int y1, uint16_t color)
{
#ifdef SDL_PC
    if (!g_device) return;
    int width = g_device->Width();
    int height = g_device->Height();
    Color col = RGB565ToColor(color);
#else
    if (!g_framebuffer) return;
    int width = DISPLAY_WIDTH;
    int height = DISPLAY_HEIGHT;
#endif

    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
#ifdef SDL_PC
            g_device->PutPixel(x0, y0, col);
#else
            g_framebuffer[y0 * DISPLAY_WIDTH + x0] = color;
#endif
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void Rasterizer_GetStats(RasterizerStats_t* stats) { *stats = g_stats; }
void Rasterizer_ResetStats(void) { memset(&g_stats, 0, sizeof(g_stats)); }