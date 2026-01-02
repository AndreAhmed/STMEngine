#ifndef RASTERIZER_H
#define RASTERIZER_H

#include <stdint.h>
#include "math3d.h"
#include "engine_config.h"

#ifdef __cplusplus
extern "C" {
#endif

    /* Screen-space vertex after projection */
    typedef struct {
        int32_t x, y;       /* 16.16 fixed point screen coords */
        float z;            /* Normalized depth [0,1] */
        float w_inv;        /* 1/w for perspective correction */
        float u, v;         /* Texture coordinates (pre-divided by w for perspective) */
        uint16_t color;     /* RGB565 vertex color/lighting */
    } ScreenVertex_t;

    /* Texture structure */
    typedef struct {
        uint16_t* pixels;
        uint16_t width;
        uint16_t height;
        uint16_t width_mask;    /* width - 1 for power-of-2 wrapping */
        uint16_t height_mask;   /* height - 1 for power-of-2 wrapping */
    } Texture_t;

    /* Rasterizer statistics */
    typedef struct {
        uint32_t triangles_submitted;
        uint32_t triangles_culled;
        uint32_t triangles_drawn;
        uint32_t pixels_drawn;
    } RasterizerStats_t;

    /* Initialization */
    void Rasterizer_Init(void);

    /* Platform-specific setup */
#ifdef SDL_PC
#ifdef __cplusplus
    class Device;
    void Rasterizer_SetDevice(Device* device);
#endif
#else
    void Rasterizer_SetFrameBuffer(uint16_t* fb);
#endif

    /* Clear operations */
    void Rasterizer_Clear(uint16_t color);
    void Rasterizer_ClearDepth(void);

    /* Triangle rasterization */
    void Rasterizer_DrawTriangle(const ScreenVertex_t* v0, const ScreenVertex_t* v1,
        const ScreenVertex_t* v2, const Texture_t* texture);
    void Rasterizer_DrawTriangleSolid(const ScreenVertex_t* v0, const ScreenVertex_t* v1,
        const ScreenVertex_t* v2, uint16_t color);

    /* Line drawing (Bresenham) */
    void Rasterizer_DrawLine(int x0, int y0, int x1, int y1, uint16_t color);

    /* Texture sampling */
    uint16_t Texture_Sample(const Texture_t* tex, float u, float v);

    /* Statistics */
    void Rasterizer_GetStats(RasterizerStats_t* stats);
    void Rasterizer_ResetStats(void);

#ifdef __cplusplus
}
#endif

#endif /* RASTERIZER_H */