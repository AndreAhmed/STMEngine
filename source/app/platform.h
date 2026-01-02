/**
 * @file platform.h
 * @brief Platform compatibility macros for MSVC/GCC/Clang
 *
 * Include order: math3d.h -> platform.h -> other headers
 */

#ifndef PLATFORM_H
#define PLATFORM_H

 /* Include engine_config.h for section macros if available */
#include "engine_config.h"

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
  * Platform Detection (engine_config.h may already define these)
  * ============================================================ */

#ifndef SDL_PC
#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#define SDL_PC 1
#elif defined(__arm__) || defined(__ARM_ARCH) || defined(STM32H7)
#define PLATFORM_EMBEDDED 1
#else
#define PLATFORM_UNKNOWN 1
#endif
#endif

  /* ============================================================
   * Memory Section Attributes (fallback if not in engine_config.h)
   * ============================================================ */

#ifndef SECTION_SDRAM
#ifdef COMPILER_MSVC
#define SDRAM_BSS
#define SECTION_SDRAM
#define SECTION_DTCM
#define ALIGN(x) __declspec(align(x))
#else
#define SDRAM_BSS       __attribute__((section(".sdram_bss")))
#define SECTION_SDRAM   __attribute__((section(".sdram")))
#define SECTION_DTCM    __attribute__((section(".dtcm")))
#define ALIGN(x) __attribute__((aligned(x)))
#endif
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
 * These work in both C and C++, with any compiler
 * ============================================================ */

 /* Forward declare types if math3d.h not included yet */
#ifndef MATH3D_H
typedef struct { float x, y; } Vec2;
typedef struct { float x, y, z; } Vec3;
typedef struct { float x, y, z, w; } Vec4;
#endif

static inline Vec2 MakeVec2(float x, float y) {
    Vec2 v; v.x = x; v.y = y; return v;
}

static inline Vec3 MakeVec3(float x, float y, float z) {
    Vec3 v; v.x = x; v.y = y; v.z = z; return v;
}

static inline Vec4 MakeVec4(float x, float y, float z, float w) {
    Vec4 v; v.x = x; v.y = y; v.z = z; v.w = w; return v;
}

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