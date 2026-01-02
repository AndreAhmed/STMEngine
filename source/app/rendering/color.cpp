#include "color.h"
#include <stdint.h>

#ifdef __cplusplus
#include <SDL/SDL.h>

Color blendAverage(const Color& source, const Color& target)
{
    Color ret;
    ret.r = ((uint32_t)source.r + (uint32_t)target.r) >> 1;
    ret.g = ((uint32_t)source.g + (uint32_t)target.g) >> 1;
    ret.b = ((uint32_t)source.b + (uint32_t)target.b) >> 1;
    ret.a = 255;
    return ret;
}

Color blendMultiply(const Color& source, const Color& target)
{
    Color ret;
    ret.r = ((uint32_t)source.r * (uint32_t)target.r) >> 8;
    ret.g = ((uint32_t)source.g * (uint32_t)target.g) >> 8;
    ret.b = ((uint32_t)source.b * (uint32_t)target.b) >> 8;
    ret.a = 255;
    return ret;
}

Color blendAdd(const Color& source, const Color& target)
{
    uint32_t r = (uint32_t)source.r + (uint32_t)target.r;
    uint32_t g = (uint32_t)source.g + (uint32_t)target.g;
    uint32_t b = (uint32_t)source.b + (uint32_t)target.b;
    Color ret;
    ret.r = (r > 255) ? 255 : (uint8_t)r;
    ret.g = (g > 255) ? 255 : (uint8_t)g;
    ret.b = (b > 255) ? 255 : (uint8_t)b;
    ret.a = 255;
    return ret;
}

#else /* C version */

Color blendAverage(const Color* source, const Color* target)
{
    Color ret;
    ret.r = ((uint32_t)source->r + (uint32_t)target->r) >> 1;
    ret.g = ((uint32_t)source->g + (uint32_t)target->g) >> 1;
    ret.b = ((uint32_t)source->b + (uint32_t)target->b) >> 1;
    ret.a = 255;
    return ret;
}

Color blendMultiply(const Color* source, const Color* target)
{
    Color ret;
    ret.r = ((uint32_t)source->r * (uint32_t)target->r) >> 8;
    ret.g = ((uint32_t)source->g * (uint32_t)target->g) >> 8;
    ret.b = ((uint32_t)source->b * (uint32_t)target->b) >> 8;
    ret.a = 255;
    return ret;
}

Color blendAdd(const Color* source, const Color* target)
{
    uint32_t r = (uint32_t)source->r + (uint32_t)target->r;
    uint32_t g = (uint32_t)source->g + (uint32_t)target->g;
    uint32_t b = (uint32_t)source->b + (uint32_t)target->b;
    Color ret;
    ret.r = (r > 255) ? 255 : (uint8_t)r;
    ret.g = (g > 255) ? 255 : (uint8_t)g;
    ret.b = (b > 255) ? 255 : (uint8_t)b;
    ret.a = 255;
    return ret;
}

#endif /* __cplusplus */