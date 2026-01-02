/**
 * @file platform.h
 * @brief Platform compatibility macros for MSVC/GCC/Clang
 */

#ifndef PLATFORM_H
#define PLATFORM_H

 /* ============================================================
  * Compiler Detection
  * ============================================================ */

#if defined(_MSC_VER)
#define COMPILER_MSVC 1
#elif defined(__GNUC__)
#define COMPILER_GCC 1
#elif defined(__clang__)
#define COMPILER_CLANG 1
#endif

  /* ============================================================
   * Platform Detection
   * ============================================================ */

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#ifndef SDL_PC
#define SDL_PC 1
#endif
#elif defined(__arm__) || defined(__ARM_ARCH) || defined(STM32H7)
#define PLATFORM_EMBEDDED 1
#else
#define PLATFORM_UNKNOWN 1
#endif

   /* ============================================================
    * Memory Section Attributes
    * GCC/Clang use __attribute__, MSVC uses #pragma or nothing
    * ============================================================ */

#ifdef COMPILER_MSVC
    /* MSVC: No section attributes on PC - just regular memory */
#define SDRAM_BSS
#define SDRAM_DATA
#define DTCM_BSS
#define DTCM_DATA
#define FRAMEBUFFER
#define SECTION_SDRAM
#define SECTION_DTCM

/* Alignment */
#define ALIGN(x) __declspec(align(x))
#else
    /* GCC/Clang for embedded */
#define SDRAM_BSS       __attribute__((section(".sdram_bss")))
#define SDRAM_DATA      __attribute__((section(".sdram_data")))
#define DTCM_BSS        __attribute__((section(".dtcm_bss")))
#define DTCM_DATA       __attribute__((section(".dtcm_data")))
#define FRAMEBUFFER     __attribute__((section(".framebuffer")))
#define SECTION_SDRAM   __attribute__((section(".sdram")))
#define SECTION_DTCM    __attribute__((section(".dtcm")))

/* Alignment */
#define ALIGN(x) __attribute__((aligned(x)))
#endif

/* ============================================================
 * Struct Initialization Helpers (for C++ compatibility)
 * MSVC C++ doesn't support C99 compound literals like (Vec3){1,2,3}
 * ============================================================ */

#ifdef __cplusplus
 /* C++ style initialization */
#define VEC2_INIT(x_, y_)           Vec2{x_, y_}
#define VEC3_INIT(x_, y_, z_)       Vec3{x_, y_, z_}
#define VEC4_INIT(x_, y_, z_, w_)   Vec4{x_, y_, z_, w_}
#define VERTEX_INIT(pos, norm, uv)  Vertex_t{pos, norm, uv}
#else
 /* C99 compound literals */
#define VEC2_INIT(x_, y_)           (Vec2){x_, y_}
#define VEC3_INIT(x_, y_, z_)       (Vec3){x_, y_, z_}
#define VEC4_INIT(x_, y_, z_, w_)   (Vec4){x_, y_, z_, w_}
#define VERTEX_INIT(pos, norm, uv)  (Vertex_t){pos, norm, uv}
#endif

/* ============================================================
 * Inline Functions for Struct Creation (safest approach)
 * Include math3d.h before platform.h to use these
 * ============================================================ */

#ifdef MATH3D_H
static inline Vec2 MakeVec2(float x, float y) {
    Vec2 v; v.x = x; v.y = y; return v;
}

static inline Vec3 MakeVec3(float x, float y, float z) {
    Vec3 v; v.x = x; v.y = y; v.z = z; return v;
}

static inline Vec4 MakeVec4(float x, float y, float z, float w) {
    Vec4 v; v.x = x; v.y = y; v.z = z; v.w = w; return v;
}
#endif

/* ============================================================
 * Memory Allocation (C++ safe)
 * ============================================================ */

#ifdef __cplusplus
#include <cstdlib>
#include <cstring>
#include <cstdint>

#define ALLOC(type, count)  static_cast<type*>(malloc((count) * sizeof(type)))
#define FREE(ptr)           free(ptr)
#else
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ALLOC(type, count)  ((type*)malloc((count) * sizeof(type)))
#define FREE(ptr)           free(ptr)
#endif

#endif /* PLATFORM_H */