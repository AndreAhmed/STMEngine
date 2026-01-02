/**
 * @file mesh.h
 * @brief Static Memory Pool Based Mesh System - NO MALLOC
 */
#ifndef MESH_H
#define MESH_H

#include <stdint.h>
#include "math3d.h"
#include "engine_config.h"

#ifdef __cplusplus
extern "C" {
#endif

    /* Pool sizes - adjust based on your SRAM budget */
#define MAX_TOTAL_VERTICES      40960
#define MAX_TOTAL_INDICES       81920
#define MAX_MD2_FRAMES          200
#define MAX_MD2_VERTICES        204800

/* Vertex formats */
    typedef struct {
        Vec3 position;
        Vec3 normal;
        Vec2 texcoord;
    } Vertex_t;

    typedef struct {
        uint8_t x, y, z;
        uint8_t normal_index;
    } MD2Vertex_t;

    /* Static mesh descriptor */
    typedef struct {
        uint16_t vertex_start;
        uint16_t vertex_count;
        uint16_t index_start;
        uint16_t index_count;
        Vec3 bounds_center;
        float bounds_radius;
    } StaticMeshDesc_t;

    /* MD2 frame descriptor */
    typedef struct {
        Vec3 scale;
        Vec3 translate;
        uint16_t vertex_start;
        uint16_t vertex_count;
    } MD2FrameDesc_t;


    typedef struct {
        float u, v;
    } MD2UV_t;
   
	// Add to global pools (near the other pools)
#ifdef SDL_PC
	extern MD2UV_t g_md2_uv_pool[MAX_MD2_VERTICES];
#else
	extern MD2UV_t g_md2_uv_pool[MAX_MD2_VERTICES] SECTION_SDRAM;
#endif

	extern uint32_t g_md2_uv_used;

    // Add allocator function
    uint32_t AllocMD2UVs(uint32_t count);
    typedef struct {
        uint16_t frame_start;
        uint16_t frame_count;
        uint16_t index_start;
        uint16_t index_count;
        uint16_t verts_per_frame;
        uint16_t uv_start;     
        uint16_t uv_count;   
    } AnimatedMeshDesc_t;

    /* Mesh slot */
    typedef struct {
        uint8_t type;           /* 0=free, 1=static, 2=animated */
        uint8_t flags;
        union {
            StaticMeshDesc_t stat;
            AnimatedMeshDesc_t anim;
        };
    } MeshSlot_t;

    /* ============================================================
     * Pool Globals (defined in mesh.cpp)
     * ============================================================ */
    extern Vertex_t g_vertex_pool[];
    extern uint16_t g_index_pool[];
    extern MD2FrameDesc_t g_frame_pool[];
    extern MD2Vertex_t g_md2_vertex_pool[];
    extern MeshSlot_t g_meshes[];

    /* ============================================================
     * Pool Allocators (defined in mesh.cpp)
     * ============================================================ */
    uint32_t AllocMeshSlot(void);
    uint32_t AllocVertices(uint32_t count);
    uint32_t AllocIndices(uint32_t count);
    uint32_t AllocFrames(uint32_t count);
    uint32_t AllocMD2Vertices(uint32_t count);

    /* ============================================================
     * Public API
     * ============================================================ */
    void Mesh_Init(void);
    uint32_t Mesh_LoadOBJ(const void* data, uint32_t size);
    uint32_t Mesh_LoadMD2(const void* data, uint32_t size);
    uint32_t Mesh_CreateCube(float size);
    uint32_t Mesh_CreatePlane(float w, float h);

    MeshSlot_t* Mesh_Get(uint32_t id);
    Vertex_t* Mesh_GetVertexPtr(uint32_t start);
    uint16_t* Mesh_GetIndexPtr(uint32_t start);
    MD2FrameDesc_t* Mesh_GetFramePtr(uint32_t start);
    MD2Vertex_t* Mesh_GetMD2VertexPtr(uint32_t start);

    void Mesh_Free(uint32_t id);
    uint32_t Mesh_GetFreeVertexCount(void);
    uint32_t Mesh_GetFreeIndexCount(void);

    /* MD2 Animation */
    void Mesh_GetMD2InterpolatedVertex(uint32_t mesh_id, uint32_t vertex_index,
        uint16_t frame_a, uint16_t frame_b, float lerp,
        Vec3* out_position, Vec3* out_normal);

    void Mesh_GetMD2Vertex(uint32_t mesh_id, uint32_t vert_idx,
        uint32_t frame_a, uint32_t frame_b, float t,
        Vec3* pos, Vec3* norm, Vec2* uv);

    int MD2_GetAnimRange(const char* name, int* start, int* end);

#ifdef __cplusplus
}
#endif

#endif /* MESH_H */