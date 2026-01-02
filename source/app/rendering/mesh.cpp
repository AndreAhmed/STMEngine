/**
 * @file mesh.cpp
 * @brief Static Memory Pool Mesh System - NO MALLOC
 *
 * All memory is statically allocated at compile time.
 * Pools are placed in specific memory sections for optimal access.
 */

#include "mesh.h"
#include "platform.h"
#include <string.h>
#include <stdlib.h>

 /* ============================================================
  * Static Memory Pools
  * On embedded: placed in SDRAM via linker script
  * On PC: regular static memory
  * ============================================================ */
  
 
#ifdef SDL_PC
MD2UV_t g_md2_uv_pool[MAX_MD2_VERTICES];
#else
MD2UV_t g_md2_uv_pool[MAX_MD2_VERTICES] SECTION_SDRAM;
#endif

#ifdef SDL_PC
Vertex_t g_vertex_pool[MAX_TOTAL_VERTICES];
uint16_t g_index_pool[MAX_TOTAL_INDICES];
MD2Vertex_t g_md2_vertex_pool[MAX_MD2_VERTICES];
#else
  /* GCC embedded - use section attributes */
Vertex_t g_vertex_pool[MAX_TOTAL_VERTICES] SECTION_SDRAM;
uint16_t g_index_pool[MAX_TOTAL_INDICES] SECTION_SDRAM;
MD2Vertex_t g_md2_vertex_pool[MAX_MD2_VERTICES] SECTION_SDRAM;
#endif


uint32_t g_vertex_used = 0;
uint32_t g_index_used = 0;
uint32_t g_md2_uv_used = 0;

/* MD2 frame descriptors */
MD2FrameDesc_t g_frame_pool[MAX_MD2_FRAMES];
uint32_t g_frame_used = 0;

uint32_t g_md2_vertex_used = 0;

/* Mesh slots */
MeshSlot_t g_meshes[MAX_MESHES];

/* ============================================================
 * Pool Management
 * ============================================================ */

void Mesh_Init(void)
{
    memset(g_meshes, 0, sizeof(g_meshes));
    g_vertex_used = 0;
    g_index_used = 0;
    g_frame_used = 0;
    g_md2_vertex_used = 0;
    g_md2_uv_used = 0;
}

uint32_t Mesh_GetFreeVertexCount(void) { return MAX_TOTAL_VERTICES - g_vertex_used; }
uint32_t Mesh_GetFreeIndexCount(void) { return MAX_TOTAL_INDICES - g_index_used; }

uint32_t AllocMeshSlot(void)
{
    for (uint32_t i = 0; i < MAX_MESHES; i++) {
        if (g_meshes[i].type == 0) return i;
    }
    return 0xFFFFFFFF;
}

uint32_t AllocVertices(uint32_t count)
{
    if (g_vertex_used + count > MAX_TOTAL_VERTICES) return 0xFFFFFFFF;
    uint32_t start = g_vertex_used;
    g_vertex_used += count;
    return start;
}

uint32_t AllocIndices(uint32_t count)
{
    if (g_index_used + count > MAX_TOTAL_INDICES) return 0xFFFFFFFF;
    uint32_t start = g_index_used;
    g_index_used += count;
    return start;
}

uint32_t AllocFrames(uint32_t count)
{
    if (g_frame_used + count > MAX_MD2_FRAMES) return 0xFFFFFFFF;
    uint32_t start = g_frame_used;
    g_frame_used += count;
    return start;
}

uint32_t AllocMD2Vertices(uint32_t count)
{
    if (g_md2_vertex_used + count > MAX_MD2_VERTICES) return 0xFFFFFFFF;
    uint32_t start = g_md2_vertex_used;
    g_md2_vertex_used += count;
    return start;
}

/* ============================================================
 * Accessors
 * ============================================================ */

MeshSlot_t* Mesh_Get(uint32_t id)
{
    if (id >= MAX_MESHES || g_meshes[id].type == 0) return NULL;
    return &g_meshes[id];
}

Vertex_t* Mesh_GetVertexPtr(uint32_t start)
{
    return (start < MAX_TOTAL_VERTICES) ? &g_vertex_pool[start] : NULL;
}

uint16_t* Mesh_GetIndexPtr(uint32_t start)
{
    return (start < MAX_TOTAL_INDICES) ? &g_index_pool[start] : NULL;
}

MD2FrameDesc_t* Mesh_GetFramePtr(uint32_t start)
{
    return (start < MAX_MD2_FRAMES) ? &g_frame_pool[start] : NULL;
}

MD2Vertex_t* Mesh_GetMD2VertexPtr(uint32_t start)
{
    return (start < MAX_MD2_VERTICES) ? &g_md2_vertex_pool[start] : NULL;
}

/* ============================================================
 * Helper: Create Vertex
 * ============================================================ */

static inline Vertex_t MakeVertex(Vec3 pos, Vec3 norm, Vec2 uv)
{
    Vertex_t v;
    v.position = pos;
    v.normal = norm;
    v.texcoord = uv;
    return v;
}

/* ============================================================
 * Primitive Creation
 * ============================================================ */

uint32_t Mesh_CreateCube(float size)
{
    uint32_t slot = AllocMeshSlot();
    if (slot == 0xFFFFFFFF) return 0xFFFFFFFF;

    uint32_t v_start = AllocVertices(24);
    uint32_t i_start = AllocIndices(36);
    if (v_start == 0xFFFFFFFF || i_start == 0xFFFFFFFF) return 0xFFFFFFFF;

    float h = size * 0.5f;
    Vertex_t* v = &g_vertex_pool[v_start];

    /* Front face (+Z) */
    v[0] = MakeVertex(MakeVec3(-h, -h, h), MakeVec3(0, 0, 1), MakeVec2(0, 1));
    v[1] = MakeVertex(MakeVec3(h, -h, h), MakeVec3(0, 0, 1), MakeVec2(1, 1));
    v[2] = MakeVertex(MakeVec3(h, h, h), MakeVec3(0, 0, 1), MakeVec2(1, 0));
    v[3] = MakeVertex(MakeVec3(-h, h, h), MakeVec3(0, 0, 1), MakeVec2(0, 0));
    /* Back face (-Z) */
    v[4] = MakeVertex(MakeVec3(h, -h, -h), MakeVec3(0, 0, -1), MakeVec2(0, 1));
    v[5] = MakeVertex(MakeVec3(-h, -h, -h), MakeVec3(0, 0, -1), MakeVec2(1, 1));
    v[6] = MakeVertex(MakeVec3(-h, h, -h), MakeVec3(0, 0, -1), MakeVec2(1, 0));
    v[7] = MakeVertex(MakeVec3(h, h, -h), MakeVec3(0, 0, -1), MakeVec2(0, 0));
    /* Top (+Y) */
    v[8] = MakeVertex(MakeVec3(-h, h, h), MakeVec3(0, 1, 0), MakeVec2(0, 1));
    v[9] = MakeVertex(MakeVec3(h, h, h), MakeVec3(0, 1, 0), MakeVec2(1, 1));
    v[10] = MakeVertex(MakeVec3(h, h, -h), MakeVec3(0, 1, 0), MakeVec2(1, 0));
    v[11] = MakeVertex(MakeVec3(-h, h, -h), MakeVec3(0, 1, 0), MakeVec2(0, 0));
    /* Bottom (-Y) */
    v[12] = MakeVertex(MakeVec3(-h, -h, -h), MakeVec3(0, -1, 0), MakeVec2(0, 1));
    v[13] = MakeVertex(MakeVec3(h, -h, -h), MakeVec3(0, -1, 0), MakeVec2(1, 1));
    v[14] = MakeVertex(MakeVec3(h, -h, h), MakeVec3(0, -1, 0), MakeVec2(1, 0));
    v[15] = MakeVertex(MakeVec3(-h, -h, h), MakeVec3(0, -1, 0), MakeVec2(0, 0));
    /* Right (+X) */
    v[16] = MakeVertex(MakeVec3(h, -h, h), MakeVec3(1, 0, 0), MakeVec2(0, 1));
    v[17] = MakeVertex(MakeVec3(h, -h, -h), MakeVec3(1, 0, 0), MakeVec2(1, 1));
    v[18] = MakeVertex(MakeVec3(h, h, -h), MakeVec3(1, 0, 0), MakeVec2(1, 0));
    v[19] = MakeVertex(MakeVec3(h, h, h), MakeVec3(1, 0, 0), MakeVec2(0, 0));
    /* Left (-X) */
    v[20] = MakeVertex(MakeVec3(-h, -h, -h), MakeVec3(-1, 0, 0), MakeVec2(0, 1));
    v[21] = MakeVertex(MakeVec3(-h, -h, h), MakeVec3(-1, 0, 0), MakeVec2(1, 1));
    v[22] = MakeVertex(MakeVec3(-h, h, h), MakeVec3(-1, 0, 0), MakeVec2(1, 0));
    v[23] = MakeVertex(MakeVec3(-h, h, -h), MakeVec3(-1, 0, 0), MakeVec2(0, 0));

    uint16_t* idx = &g_index_pool[i_start];
    uint16_t indices[36] = {
        0,1,2,0,2,3, 4,5,6,4,6,7, 8,9,10,8,10,11,
        12,13,14,12,14,15, 16,17,18,16,18,19, 20,21,22,20,22,23
    };
    memcpy(idx, indices, sizeof(indices));

    g_meshes[slot].type = 1;
    g_meshes[slot].stat.vertex_start = v_start;
    g_meshes[slot].stat.vertex_count = 24;
    g_meshes[slot].stat.index_start = i_start;
    g_meshes[slot].stat.index_count = 36;
    g_meshes[slot].stat.bounds_center = MakeVec3(0, 0, 0);
    g_meshes[slot].stat.bounds_radius = h * 1.732f;

    return slot;
}

uint32_t Mesh_CreatePlane(float w, float h)
{
    uint32_t slot = AllocMeshSlot();
    if (slot == 0xFFFFFFFF) return 0xFFFFFFFF;

    uint32_t v_start = AllocVertices(4);
    uint32_t i_start = AllocIndices(6);
    if (v_start == 0xFFFFFFFF || i_start == 0xFFFFFFFF) return 0xFFFFFFFF;

    float hw = w * 0.5f, hh = h * 0.5f;
    Vertex_t* v = &g_vertex_pool[v_start];

    v[0] = MakeVertex(MakeVec3(-hw, 0, -hh), MakeVec3(0, 1, 0), MakeVec2(0, 0));
    v[1] = MakeVertex(MakeVec3(hw, 0, -hh), MakeVec3(0, 1, 0), MakeVec2(1, 0));
    v[2] = MakeVertex(MakeVec3(hw, 0, hh), MakeVec3(0, 1, 0), MakeVec2(1, 1));
    v[3] = MakeVertex(MakeVec3(-hw, 0, hh), MakeVec3(0, 1, 0), MakeVec2(0, 1));

    uint16_t* idx = &g_index_pool[i_start];
    idx[0] = 0; idx[1] = 2; idx[2] = 1;
    idx[3] = 0; idx[4] = 3; idx[5] = 2;

    g_meshes[slot].type = 1;
    g_meshes[slot].stat.vertex_start = v_start;
    g_meshes[slot].stat.vertex_count = 4;
    g_meshes[slot].stat.index_start = i_start;
    g_meshes[slot].stat.index_count = 6;
    g_meshes[slot].stat.bounds_center = MakeVec3(0, 0, 0);
    g_meshes[slot].stat.bounds_radius = (hw > hh) ? hw : hh;

    return slot;
}

/* ============================================================
 * OBJ Loader - No malloc version
 * ============================================================ */

 /* Temporary parsing buffers - static allocation */
#define OBJ_MAX_POS     2048
#define OBJ_MAX_NORM    2048
#define OBJ_MAX_UV      2048

static Vec3 s_obj_pos[OBJ_MAX_POS];
static Vec3 s_obj_norm[OBJ_MAX_NORM];
static Vec2 s_obj_uv[OBJ_MAX_UV];

static int ParseFloat(const char** s, float* out)
{
    while (**s == ' ' || **s == '\t') (*s)++;
    if (**s == '\0' || **s == '\n' || **s == '\r') return 0;
    char* end;
    *out = (float)strtod(*s, &end);
    if (end == *s) return 0;
    *s = end;
    return 1;
}

static int ParseInt(const char** s, int* out)
{
    while (**s == ' ' || **s == '\t') (*s)++;
    if (**s == '\0' || **s == '\n' || **s == '\r') return 0;
    char* end;
    *out = (int)strtol(*s, &end, 10);
    if (end == *s) return 0;
    *s = end;
    return 1;
}

static void ParseFaceIdx(const char** s, int* v, int* vt, int* vn)
{
    *v = *vt = *vn = 0;
    ParseInt(s, v);
    if (**s == '/') {
        (*s)++;
        if (**s != '/') ParseInt(s, vt);
        if (**s == '/') { (*s)++; ParseInt(s, vn); }
    }
    while (**s == ' ' || **s == '\t') (*s)++;
}

uint32_t Mesh_LoadOBJ(const void* data, uint32_t size)
{
    if (!data || size == 0) return 0xFFFFFFFF;

    uint32_t pos_count = 0, norm_count = 0, uv_count = 0;

    /* First pass: count and store raw data */
    const char* ptr = (const char*)data;
    const char* end = ptr + size;
    char line[256];

    while (ptr < end) {
        int len = 0;
        while (ptr < end && *ptr != '\n' && *ptr != '\r' && len < 254)
            line[len++] = *ptr++;
        line[len] = '\0';
        while (ptr < end && (*ptr == '\n' || *ptr == '\r')) ptr++;

        const char* lp = line;
        while (*lp == ' ' || *lp == '\t') lp++;

        if (lp[0] == 'v' && lp[1] == ' ') {
            lp += 2;
            Vec3 p;
            if (ParseFloat(&lp, &p.x) && ParseFloat(&lp, &p.y) && ParseFloat(&lp, &p.z)) {
                if (pos_count < OBJ_MAX_POS) s_obj_pos[pos_count++] = p;
            }
        }
        else if (lp[0] == 'v' && lp[1] == 't' && lp[2] == ' ') {
            lp += 3;
            Vec2 t;
            if (ParseFloat(&lp, &t.x) && ParseFloat(&lp, &t.y)) {
                t.y = 1.0f - t.y;
                if (uv_count < OBJ_MAX_UV) s_obj_uv[uv_count++] = t;
            }
        }
        else if (lp[0] == 'v' && lp[1] == 'n' && lp[2] == ' ') {
            lp += 3;
            Vec3 n;
            if (ParseFloat(&lp, &n.x) && ParseFloat(&lp, &n.y) && ParseFloat(&lp, &n.z)) {
                if (norm_count < OBJ_MAX_NORM) s_obj_norm[norm_count++] = Vec3_Normalize(n);
            }
        }
    }

    /* Count faces (second pass) */
    uint32_t face_count = 0;
    ptr = (const char*)data;
    while (ptr < end) {
        int len = 0;
        while (ptr < end && *ptr != '\n' && *ptr != '\r' && len < 254)
            line[len++] = *ptr++;
        line[len] = '\0';
        while (ptr < end && (*ptr == '\n' || *ptr == '\r')) ptr++;

        const char* lp = line;
        while (*lp == ' ' || *lp == '\t') lp++;
        if (lp[0] == 'f' && lp[1] == ' ') face_count++;
    }

    /* Estimate vertex/index needs (faces * 3 for triangles, * 2 for quads) */
    uint32_t max_verts = face_count * 4;
    uint32_t max_indices = face_count * 6;

    /* Allocate from pools */
    uint32_t slot = AllocMeshSlot();
    if (slot == 0xFFFFFFFF) return 0xFFFFFFFF;

    uint32_t v_start = AllocVertices(max_verts);
    uint32_t i_start = AllocIndices(max_indices);
    if (v_start == 0xFFFFFFFF || i_start == 0xFFFFFFFF) return 0xFFFFFFFF;

    Vertex_t* out_v = &g_vertex_pool[v_start];
    uint16_t* out_i = &g_index_pool[i_start];
    uint32_t v_count = 0, i_count = 0;

    /* Third pass: build mesh */
    ptr = (const char*)data;
    while (ptr < end) {
        int len = 0;
        while (ptr < end && *ptr != '\n' && *ptr != '\r' && len < 254)
            line[len++] = *ptr++;
        line[len] = '\0';
        while (ptr < end && (*ptr == '\n' || *ptr == '\r')) ptr++;

        const char* lp = line;
        while (*lp == ' ' || *lp == '\t') lp++;

        if (lp[0] == 'f' && lp[1] == ' ') {
            lp += 2;

            int fv[4], ft[4], fn[4];
            int fc = 0;

            while (*lp && fc < 4) {
                ParseFaceIdx(&lp, &fv[fc], &ft[fc], &fn[fc]);
                if (fv[fc] != 0) fc++;
                else break;
            }

            if (fc >= 3 && v_count + fc <= max_verts && i_count + 6 <= max_indices) {
                uint32_t base = v_count;

                for (int i = 0; i < fc; i++) {
                    int vi = (fv[i] > 0) ? fv[i] - 1 : (int)pos_count + fv[i];
                    int ti = (ft[i] > 0) ? ft[i] - 1 : (ft[i] < 0) ? (int)uv_count + ft[i] : -1;
                    int ni = (fn[i] > 0) ? fn[i] - 1 : (fn[i] < 0) ? (int)norm_count + fn[i] : -1;

                    out_v[v_count].position = (vi >= 0 && vi < (int)pos_count) ? s_obj_pos[vi] : Vec3_Zero();
                    out_v[v_count].texcoord = (ti >= 0 && ti < (int)uv_count) ? s_obj_uv[ti] : MakeVec2(0, 0);
                    out_v[v_count].normal = (ni >= 0 && ni < (int)norm_count) ? s_obj_norm[ni] : MakeVec3(0, 1, 0);
                    v_count++;
                }

                /* Triangle */
                out_i[i_count++] = (uint16_t)base;
                out_i[i_count++] = (uint16_t)(base + 1);
                out_i[i_count++] = (uint16_t)(base + 2);

                /* Quad -> second triangle */
                if (fc == 4) {
                    out_i[i_count++] = (uint16_t)base;
                    out_i[i_count++] = (uint16_t)(base + 2);
                    out_i[i_count++] = (uint16_t)(base + 3);
                }
            }
        }
    }

    /* Trim unused allocation */
    g_vertex_used = v_start + v_count;
    g_index_used = i_start + i_count;

    /* Calculate bounds */
    Vec3 bmin = out_v[0].position, bmax = out_v[0].position;
    for (uint32_t i = 1; i < v_count; i++) {
        bmin = Vec3_Min(bmin, out_v[i].position);
        bmax = Vec3_Max(bmax, out_v[i].position);
    }

    g_meshes[slot].type = 1;
    g_meshes[slot].stat.vertex_start = v_start;
    g_meshes[slot].stat.vertex_count = v_count;
    g_meshes[slot].stat.index_start = i_start;
    g_meshes[slot].stat.index_count = i_count;
    g_meshes[slot].stat.bounds_center = Vec3_Scale(Vec3_Add(bmin, bmax), 0.5f);
    g_meshes[slot].stat.bounds_radius = Vec3_Length(Vec3_Sub(bmax, g_meshes[slot].stat.bounds_center));

    return slot;
}

/* ============================================================
 * Mesh Freeing
 * ============================================================ */

void Mesh_Free(uint32_t id)
{
    if (id < MAX_MESHES) {
        g_meshes[id].type = 0;
    }
}

/* ============================================================
 * MD2 Vertex Interpolation
 * ============================================================ */

 /* MD2 pre-computed normals table (162 normals) */
static const float g_md2_normals[162][3] = {
    {-0.525731f, 0.000000f, 0.850651f}, {-0.442863f, 0.238856f, 0.864188f},
    {-0.295242f, 0.000000f, 0.955423f}, {-0.309017f, 0.500000f, 0.809017f},
    {-0.162460f, 0.262866f, 0.951056f}, {0.000000f, 0.000000f, 1.000000f},
    {0.000000f, 0.850651f, 0.525731f}, {-0.147621f, 0.716567f, 0.681718f},
    {0.147621f, 0.716567f, 0.681718f}, {0.000000f, 0.525731f, 0.850651f},
    {0.309017f, 0.500000f, 0.809017f}, {0.525731f, 0.000000f, 0.850651f},
    {0.295242f, 0.000000f, 0.955423f}, {0.442863f, 0.238856f, 0.864188f},
    {0.162460f, 0.262866f, 0.951056f}, {-0.681718f, 0.147621f, 0.716567f},
    {-0.809017f, 0.309017f, 0.500000f}, {-0.587785f, 0.425325f, 0.688191f},
    {-0.850651f, 0.525731f, 0.000000f}, {-0.864188f, 0.442863f, 0.238856f},
    {-0.716567f, 0.681718f, 0.147621f}, {-0.688191f, 0.587785f, 0.425325f},
    {-0.500000f, 0.809017f, 0.309017f}, {-0.238856f, 0.864188f, 0.442863f},
    {-0.425325f, 0.688191f, 0.587785f}, {-0.716567f, 0.681718f, -0.147621f},
    {-0.500000f, 0.809017f, -0.309017f}, {-0.525731f, 0.850651f, 0.000000f},
    {0.000000f, 0.850651f, -0.525731f}, {-0.238856f, 0.864188f, -0.442863f},
    {0.000000f, 0.955423f, -0.295242f}, {-0.262866f, 0.951056f, -0.162460f},
    {0.000000f, 1.000000f, 0.000000f}, {0.000000f, 0.955423f, 0.295242f},
    {-0.262866f, 0.951056f, 0.162460f}, {0.238856f, 0.864188f, 0.442863f},
    {0.262866f, 0.951056f, 0.162460f}, {0.500000f, 0.809017f, 0.309017f},
    {0.238856f, 0.864188f, -0.442863f}, {0.262866f, 0.951056f, -0.162460f},
    {0.500000f, 0.809017f, -0.309017f}, {0.850651f, 0.525731f, 0.000000f},
    {0.716567f, 0.681718f, 0.147621f}, {0.716567f, 0.681718f, -0.147621f},
    {0.525731f, 0.850651f, 0.000000f}, {0.425325f, 0.688191f, 0.587785f},
    {0.864188f, 0.442863f, 0.238856f}, {0.688191f, 0.587785f, 0.425325f},
    {0.809017f, 0.309017f, 0.500000f}, {0.681718f, 0.147621f, 0.716567f},
    {0.587785f, 0.425325f, 0.688191f}, {0.955423f, 0.295242f, 0.000000f},
    {1.000000f, 0.000000f, 0.000000f}, {0.951056f, 0.162460f, 0.262866f},
    {0.850651f, -0.525731f, 0.000000f}, {0.955423f, -0.295242f, 0.000000f},
    {0.864188f, -0.442863f, 0.238856f}, {0.951056f, -0.162460f, 0.262866f},
    {0.809017f, -0.309017f, 0.500000f}, {0.681718f, -0.147621f, 0.716567f},
    {0.850651f, 0.000000f, 0.525731f}, {0.864188f, 0.442863f, -0.238856f},
    {0.809017f, 0.309017f, -0.500000f}, {0.951056f, 0.162460f, -0.262866f},
    {0.525731f, 0.000000f, -0.850651f}, {0.681718f, 0.147621f, -0.716567f},
    {0.681718f, -0.147621f, -0.716567f}, {0.850651f, 0.000000f, -0.525731f},
    {0.809017f, -0.309017f, -0.500000f}, {0.864188f, -0.442863f, -0.238856f},
    {0.951056f, -0.162460f, -0.262866f}, {0.147621f, 0.716567f, -0.681718f},
    {0.309017f, 0.500000f, -0.809017f}, {0.425325f, 0.688191f, -0.587785f},
    {0.442863f, 0.238856f, -0.864188f}, {0.587785f, 0.425325f, -0.688191f},
    {0.688191f, 0.587785f, -0.425325f}, {-0.147621f, 0.716567f, -0.681718f},
    {-0.309017f, 0.500000f, -0.809017f}, {0.000000f, 0.525731f, -0.850651f},
    {-0.525731f, 0.000000f, -0.850651f}, {-0.442863f, 0.238856f, -0.864188f},
    {-0.295242f, 0.000000f, -0.955423f}, {-0.162460f, 0.262866f, -0.951056f},
    {0.000000f, 0.000000f, -1.000000f}, {0.295242f, 0.000000f, -0.955423f},
    {0.162460f, 0.262866f, -0.951056f}, {-0.442863f, -0.238856f, -0.864188f},
    {-0.309017f, -0.500000f, -0.809017f}, {-0.162460f, -0.262866f, -0.951056f},
    {0.000000f, -0.850651f, -0.525731f}, {-0.147621f, -0.716567f, -0.681718f},
    {0.147621f, -0.716567f, -0.681718f}, {0.000000f, -0.525731f, -0.850651f},
    {0.309017f, -0.500000f, -0.809017f}, {0.442863f, -0.238856f, -0.864188f},
    {0.162460f, -0.262866f, -0.951056f}, {0.238856f, -0.864188f, -0.442863f},
    {0.500000f, -0.809017f, -0.309017f}, {0.425325f, -0.688191f, -0.587785f},
    {0.716567f, -0.681718f, -0.147621f}, {0.688191f, -0.587785f, -0.425325f},
    {0.587785f, -0.425325f, -0.688191f}, {0.000000f, -0.955423f, -0.295242f},
    {0.000000f, -1.000000f, 0.000000f}, {0.262866f, -0.951056f, -0.162460f},
    {0.000000f, -0.850651f, 0.525731f}, {0.000000f, -0.955423f, 0.295242f},
    {0.238856f, -0.864188f, 0.442863f}, {0.262866f, -0.951056f, 0.162460f},
    {0.500000f, -0.809017f, 0.309017f}, {0.716567f, -0.681718f, 0.147621f},
    {0.525731f, -0.850651f, 0.000000f}, {-0.238856f, -0.864188f, -0.442863f},
    {-0.500000f, -0.809017f, -0.309017f}, {-0.262866f, -0.951056f, -0.162460f},
    {-0.850651f, -0.525731f, 0.000000f}, {-0.716567f, -0.681718f, -0.147621f},
    {-0.716567f, -0.681718f, 0.147621f}, {-0.525731f, -0.850651f, 0.000000f},
    {-0.500000f, -0.809017f, 0.309017f}, {-0.238856f, -0.864188f, 0.442863f},
    {-0.262866f, -0.951056f, 0.162460f}, {-0.864188f, -0.442863f, 0.238856f},
    {-0.809017f, -0.309017f, 0.500000f}, {-0.688191f, -0.587785f, 0.425325f},
    {-0.681718f, -0.147621f, 0.716567f}, {-0.442863f, -0.238856f, 0.864188f},
    {-0.587785f, -0.425325f, 0.688191f}, {-0.309017f, -0.500000f, 0.809017f},
    {-0.147621f, -0.716567f, 0.681718f}, {-0.425325f, -0.688191f, 0.587785f},
    {-0.162460f, -0.262866f, 0.951056f}, {0.442863f, -0.238856f, 0.864188f},
    {0.162460f, -0.262866f, 0.951056f}, {0.309017f, -0.500000f, 0.809017f},
    {0.147621f, -0.716567f, 0.681718f}, {0.000000f, -0.525731f, 0.850651f},
    {0.425325f, -0.688191f, 0.587785f}, {0.587785f, -0.425325f, 0.688191f},
    {0.688191f, -0.587785f, 0.425325f}, {-0.955423f, 0.295242f, 0.000000f},
    {-0.951056f, 0.162460f, 0.262866f}, {-1.000000f, 0.000000f, 0.000000f},
    {-0.850651f, 0.000000f, 0.525731f}, {-0.955423f, -0.295242f, 0.000000f},
    {-0.951056f, -0.162460f, 0.262866f}, {-0.864188f, 0.442863f, -0.238856f},
    {-0.951056f, 0.162460f, -0.262866f}, {-0.809017f, 0.309017f, -0.500000f},
    {-0.864188f, -0.442863f, -0.238856f}, {-0.951056f, -0.162460f, -0.262866f},
    {-0.809017f, -0.309017f, -0.500000f}, {-0.681718f, 0.147621f, -0.716567f},
    {-0.681718f, -0.147621f, -0.716567f}, {-0.850651f, 0.000000f, -0.525731f},
    {-0.688191f, 0.587785f, -0.425325f}, {-0.587785f, 0.425325f, -0.688191f},
    {-0.425325f, 0.688191f, -0.587785f}, {-0.425325f, -0.688191f, -0.587785f},
    {-0.587785f, -0.425325f, -0.688191f}, {-0.688191f, -0.587785f, -0.425325f}
};

void Mesh_GetMD2InterpolatedVertex(uint32_t mesh_id, uint32_t vertex_index,
    uint16_t frame_a, uint16_t frame_b, float lerp,
    Vec3* out_position, Vec3* out_normal)
{
    if (mesh_id >= MAX_MESHES || g_meshes[mesh_id].type != 2) {
        *out_position = Vec3_Zero();
        *out_normal = MakeVec3(0, 1, 0);
        return;
    }

    AnimatedMeshDesc_t* anim = &g_meshes[mesh_id].anim;

    /* Clamp frames */
    if (frame_a >= anim->frame_count) frame_a = (uint16_t)(anim->frame_count - 1);
    if (frame_b >= anim->frame_count) frame_b = (uint16_t)(anim->frame_count - 1);
    if (vertex_index >= anim->verts_per_frame) {
        *out_position = Vec3_Zero();
        *out_normal = MakeVec3(0, 1, 0);
        return;
    }

    /* Get frame descriptors */
    MD2FrameDesc_t* fd_a = &g_frame_pool[anim->frame_start + frame_a];
    MD2FrameDesc_t* fd_b = &g_frame_pool[anim->frame_start + frame_b];

    /* Get compressed vertices */
    MD2Vertex_t* v_a = &g_md2_vertex_pool[fd_a->vertex_start + vertex_index];
    MD2Vertex_t* v_b = &g_md2_vertex_pool[fd_b->vertex_start + vertex_index];

    /* Decompress positions */
    Vec3 pos_a = MakeVec3(
        v_a->x * fd_a->scale.x + fd_a->translate.x,
        v_a->y * fd_a->scale.y + fd_a->translate.y,
        v_a->z * fd_a->scale.z + fd_a->translate.z
    );
    Vec3 pos_b = MakeVec3(
        v_b->x * fd_b->scale.x + fd_b->translate.x,
        v_b->y * fd_b->scale.y + fd_b->translate.y,
        v_b->z * fd_b->scale.z + fd_b->translate.z
    );

    /* Get normals from lookup table */
    int ni_a = v_a->normal_index % 162;
    int ni_b = v_b->normal_index % 162;
    Vec3 norm_a = MakeVec3(g_md2_normals[ni_a][0], g_md2_normals[ni_a][1], g_md2_normals[ni_a][2]);
    Vec3 norm_b = MakeVec3(g_md2_normals[ni_b][0], g_md2_normals[ni_b][1], g_md2_normals[ni_b][2]);

    /* Interpolate */
    *out_position = Vec3_Lerp(pos_a, pos_b, lerp);
    *out_normal = Vec3_Normalize(Vec3_Lerp(norm_a, norm_b, lerp));
}