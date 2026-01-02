/**
 * @file engine_config.h
 * @brief Engine Configuration and Memory Layout
 */
#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Platform detection */
#ifndef SDL_PC
#if defined(_WIN32) || defined(__APPLE__) || (defined(__linux__) && !defined(__arm__))
#define SDL_PC  1
#define STM32   0
#else
#define SDL_PC  0
#define STM32   1
#endif
#endif

/* Display */
#define DISPLAY_WIDTH           1240
#define DISPLAY_HEIGHT          680
#define DISPLAY_BPP             16
#define FRAMEBUFFER_SIZE        (DISPLAY_WIDTH * DISPLAY_HEIGHT * 2)

/* LTDC Timing (STM32 only) */
#define LTDC_HSYNC              1
#define LTDC_HBP                46
#define LTDC_HFP                210
#define LTDC_VSYNC              1
#define LTDC_VBP                23
#define LTDC_VFP                22

/* Memory Map (STM32 physical addresses) */
#define DTCM_BASE               0x20000000
#define DTCM_SIZE               (128 * 1024)
#define AXI_SRAM_BASE           0x24000000
#define AXI_SRAM_SIZE           (512 * 1024)
#define SRAM1_BASE              0x30000000
#define SRAM1_SIZE              (128 * 1024)
#define SRAM2_BASE              0x30020000
#define SRAM2_SIZE              (128 * 1024)
#define SRAM3_BASE              0x30040000
#define SRAM3_SIZE              (32 * 1024)
#define SRAM4_BASE              0x38000000
#define SRAM4_SIZE              (64 * 1024)
#define SDRAM_BASE              0xD0000000
#define SDRAM_SIZE              (16 * 1024 * 1024)

/* SDRAM Layout (STM32 only) */
#define FRAMEBUFFER_A_ADDR      (SDRAM_BASE + 0x00000000)
#define FRAMEBUFFER_B_ADDR      (SDRAM_BASE + 0x000C0000)
#define TEXTURE_MEM_ADDR        (SDRAM_BASE + 0x00180000)
#define MODEL_MEM_ADDR          (SDRAM_BASE + 0x00580000)

/* Engine Limits */
#define MAX_ENTITIES            256
#define MAX_RENDER_ENTITIES     256
#define MAX_MESHES              64
#define MAX_TEXTURES            64
#define MAX_MATERIALS           64
#define MAX_LIGHTS              8
#define MAX_RIGIDBODIES         128
#define MAX_CONTACTS            256
#define MAX_SOUNDS              32
#define AUDIO_CHANNELS          8
#define AUDIO_BUFFER_SIZE       1024
#define MAX_TOUCH_POINTS        5

/* Physics */
#define PHYSICS_TIMESTEP        (1.0f / 60.0f)
#define GRAVITY_Y               -9.81f

/* HSEM IDs (STM32 dual-core sync) */
#define HSEM_ID_BOOT            0
#define HSEM_ID_SCENE_BUFFER    1
#define HSEM_ID_AUDIO_BUFFER    2
#define HSEM_ID_INPUT_STATE     3

/* Flags */
#define FLAG_CM4_READY          0
#define FLAG_NEW_FRAME          1
#define MAX_FLAGS               16

/* ============================================================
 * Section Attributes - Platform Specific
 * ============================================================ */
#if defined(_MSC_VER)
 /* MSVC: No section attributes on PC, use regular memory */
#define DTCM_DATA
#define DTCM_BSS
#define AXI_DATA
#define SHARED_DATA
#define SDRAM_DATA
#define FRAMEBUFFER
#define CACHE_ALIGNED       __declspec(align(32))
#define SECTION_SDRAM
#define SECTION_DTCM
#elif defined(__GNUC__) && SDL_PC
 /* GCC on PC: No section attributes needed */
#define DTCM_DATA
#define DTCM_BSS
#define AXI_DATA
#define SHARED_DATA
#define SDRAM_DATA
#define FRAMEBUFFER
#define CACHE_ALIGNED       __attribute__((aligned(32)))
#define SECTION_SDRAM
#define SECTION_DTCM
#elif defined(__GNUC__)
 /* GCC on embedded STM32: Use linker sections */
#define DTCM_DATA           __attribute__((section(".dtcm")))
#define DTCM_BSS            __attribute__((section(".dtcm_bss")))
#define AXI_DATA            __attribute__((section(".axi_sram")))
#define SHARED_DATA         __attribute__((section(".shared"), aligned(32)))
#define SDRAM_DATA          __attribute__((section(".sdram")))
#define FRAMEBUFFER         __attribute__((section(".framebuffer"), aligned(32)))
#define CACHE_ALIGNED       __attribute__((aligned(32)))
#define SECTION_SDRAM       __attribute__((section(".sdram")))
#define SECTION_DTCM        __attribute__((section(".dtcm_bss")))
#else
 /* Unknown compiler: empty macros */
#define DTCM_DATA
#define DTCM_BSS
#define AXI_DATA
#define SHARED_DATA
#define SDRAM_DATA
#define FRAMEBUFFER
#define CACHE_ALIGNED
#define SECTION_SDRAM
#define SECTION_DTCM
#endif

/* Colors RGB565 */
#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_BLUE          0x001F
#define COLOR_YELLOW        0xFFE0
#define COLOR_CYAN          0x07FF
#define COLOR_MAGENTA       0xF81F
#define COLOR_DARK_BLUE     0x0010

#define RGB565(r,g,b) ((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|((b)>>3))

/* Utility Macros */
#define MIN(a,b)            ((a)<(b)?(a):(b))
#define MAX(a,b)            ((a)>(b)?(a):(b))
#define CLAMP(x,lo,hi)      MIN(MAX(x,lo),hi)
#define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof((arr)[0]))

#ifdef __cplusplus
}
#endif

#endif /* ENGINE_CONFIG_H */