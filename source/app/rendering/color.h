#ifndef RENDERING_COLOR_H
#define RENDERING_COLOR_H

#include <stdint.h>

#ifdef __cplusplus

struct Color
{
    Color() : r(0), g(0), b(0), a(255) {}

    Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255)
        : r(_r), g(_g), b(_b), a(_a) {
    }

    Color(uint32_t argb)
    {
        r = (argb >> 16) & 255;
        g = (argb >> 8) & 255;
        b = argb & 255;
        a = (argb >> 24) & 255;
    }

    void operator*=(float value)
    {
        r = (uint8_t)(r * value);
        g = (uint8_t)(g * value);
        b = (uint8_t)(b * value);
    }

    friend Color operator*(const Color& c, float value)
    {
        return Color((uint8_t)(c.r * value), (uint8_t)(c.g * value), (uint8_t)(c.b * value), c.a);
    }

    friend bool operator==(const Color& c1, const Color& c2)
    {
        return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a;
    }

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

Color blendAverage(const Color& source, const Color& target);
Color blendMultiply(const Color& source, const Color& target);
Color blendAdd(const Color& source, const Color& target);

#else /* C version */

typedef struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

static inline Color Color_Create(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    Color c; c.r = r; c.g = g; c.b = b; c.a = a; return c;
}

static inline Color Color_Default(void) {
    Color c; c.r = 0; c.g = 0; c.b = 0; c.a = 255; return c;
}

static inline Color Color_FromARGB(uint32_t argb) {
    Color c;
    c.r = (argb >> 16) & 255;
    c.g = (argb >> 8) & 255;
    c.b = argb & 255;
    c.a = (argb >> 24) & 255;
    return c;
}

static inline Color Color_Scale(Color c, float value) {
    Color ret;
    ret.r = (uint8_t)(c.r * value);
    ret.g = (uint8_t)(c.g * value);
    ret.b = (uint8_t)(c.b * value);
    ret.a = c.a;
    return ret;
}

static inline int Color_Equal(Color c1, Color c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a;
}

Color blendAverage(const Color* source, const Color* target);
Color blendMultiply(const Color* source, const Color* target);
Color blendAdd(const Color* source, const Color* target);

#endif /* __cplusplus */

#endif /* RENDERING_COLOR_H */