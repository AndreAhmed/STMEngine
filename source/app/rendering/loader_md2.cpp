/**
 * @file loader_md2.cpp
 * @brief MD2 Animated Model Loader - NO MALLOC
 */

#include "mesh.h"
#include "platform.h"
#include <string.h>

 /* MD2 Header */
typedef struct {
    int32_t magic;
    int32_t version;
    int32_t skin_width;
    int32_t skin_height;
    int32_t frame_size;
    int32_t num_skins;
    int32_t num_vertices;
    int32_t num_texcoords;
    int32_t num_triangles;
    int32_t num_glcmds;
    int32_t num_frames;
    int32_t offset_skins;
    int32_t offset_texcoords;
    int32_t offset_triangles;
    int32_t offset_frames;
    int32_t offset_glcmds;
    int32_t offset_end;
} MD2Header_t;

typedef struct { int16_t s, t; } MD2TexCoord_t;
typedef struct { uint16_t vertex[3]; uint16_t texcoord[3]; } MD2Triangle_t;
typedef struct { uint8_t v[3]; uint8_t normal_idx; } MD2FrameVert_t;

typedef struct {
    float scale[3];
    float translate[3];
    char name[16];
    MD2FrameVert_t verts[1];
} MD2FrameData_t;

/* Full MD2 normal table (162 normals) */
static const float g_normals[162][3] = {
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


uint32_t AllocMD2UVs(uint32_t count)
{
    if (g_md2_uv_used + count > MAX_MD2_VERTICES) return 0xFFFFFFFF;
    uint32_t start = g_md2_uv_used;
    g_md2_uv_used += count;
    return start;
}
/* Pool allocators and globals are declared in mesh.h */

uint32_t Mesh_LoadMD2(const void* data, uint32_t size)
{
    if (!data || size < sizeof(MD2Header_t)) return 0xFFFFFFFF;

    const uint8_t* ptr = (const uint8_t*)data;
    const MD2Header_t* hdr = (const MD2Header_t*)ptr;

    if (hdr->magic != 844121161 || hdr->version != 8) return 0xFFFFFFFF;
    if (hdr->num_frames > MAX_MD2_FRAMES) return 0xFFFFFFFF;

    uint32_t slot = AllocMeshSlot();
    if (slot == 0xFFFFFFFF) return 0xFFFFFFFF;

    // MD2 uses per-triangle UVs, so we need num_triangles * 3 UVs
    uint32_t num_uvs = hdr->num_triangles * 3;
    uint32_t uv_start = AllocMD2UVs(num_uvs);
    if (uv_start == 0xFFFFFFFF) return 0xFFFFFFFF;

    // Extract texture coordinates from MD2
    const MD2TexCoord_t* src_uvs = (const MD2TexCoord_t*)(ptr + hdr->offset_texcoords);
    const MD2Triangle_t* tris = (const MD2Triangle_t*)(ptr + hdr->offset_triangles);

    MD2UV_t* out_uvs = &g_md2_uv_pool[uv_start];
    float inv_w = 1.0f / (float)hdr->skin_width;
    float inv_h = 1.0f / (float)hdr->skin_height;

    // Build index buffer and expand UVs per triangle
    uint32_t idx_start = AllocIndices(hdr->num_triangles * 3);
    if (idx_start == 0xFFFFFFFF) return 0xFFFFFFFF;

    uint16_t* out_idx = &g_index_pool[idx_start];
    for (int32_t i = 0; i < hdr->num_triangles; i++) {
        // Store vertex indices
        out_idx[i * 3 + 0] = tris[i].vertex[0];
        out_idx[i * 3 + 1] = tris[i].vertex[2];
        out_idx[i * 3 + 2] = tris[i].vertex[1];

        // Store corresponding UVs (expanded per triangle corner)
        for (int j = 0; j < 3; j++) {
            uint16_t uv_idx = tris[i].texcoord[j];
            out_uvs[i * 3 + j].u = (float)src_uvs[uv_idx].s * inv_w;
            out_uvs[i * 3 + j].v = (float)src_uvs[uv_idx].t * inv_h;
        }
    }

    // Allocate frames and vertices
    uint32_t frame_start = AllocFrames(hdr->num_frames);
    if (frame_start == 0xFFFFFFFF) return 0xFFFFFFFF;

    uint32_t total_verts = hdr->num_frames * hdr->num_vertices;
    uint32_t vert_start = AllocMD2Vertices(total_verts);
    if (vert_start == 0xFFFFFFFF) return 0xFFFFFFFF;

    // Extract frame data
    const uint8_t* frame_ptr = ptr + hdr->offset_frames;
    MD2Vertex_t* out_verts = &g_md2_vertex_pool[vert_start];
    MD2FrameDesc_t* out_frames = &g_frame_pool[frame_start];

    for (int32_t f = 0; f < hdr->num_frames; f++) {
        const MD2FrameData_t* src = (const MD2FrameData_t*)frame_ptr;

        out_frames[f].scale = MakeVec3(src->scale[0], src->scale[1], src->scale[2]);
        out_frames[f].translate = MakeVec3(src->translate[0], src->translate[1], src->translate[2]);
        out_frames[f].vertex_start = (uint16_t)(vert_start + f * hdr->num_vertices);
        out_frames[f].vertex_count = (uint16_t)hdr->num_vertices;

        MD2Vertex_t* dst_v = &out_verts[f * hdr->num_vertices];
        for (int32_t v = 0; v < hdr->num_vertices; v++) {
            dst_v[v].x = src->verts[v].v[0];
            dst_v[v].y = src->verts[v].v[1];
            dst_v[v].z = src->verts[v].v[2];
            dst_v[v].normal_index = src->verts[v].normal_idx;
        }

        frame_ptr += hdr->frame_size;
    }

    g_meshes[slot].type = 2;
    g_meshes[slot].anim.frame_start = (uint16_t)frame_start;
    g_meshes[slot].anim.frame_count = (uint16_t)hdr->num_frames;
    g_meshes[slot].anim.index_start = (uint16_t)idx_start;
    g_meshes[slot].anim.index_count = (uint16_t)(hdr->num_triangles * 3);
    g_meshes[slot].anim.verts_per_frame = (uint16_t)hdr->num_vertices;
    g_meshes[slot].anim.uv_start = (uint16_t)uv_start;
    g_meshes[slot].anim.uv_count = (uint16_t)num_uvs;

    return slot;
}
void Mesh_GetMD2Vertex(uint32_t mesh_id, uint32_t vert_idx,
    uint32_t frame_a, uint32_t frame_b, float t,
    Vec3* pos, Vec3* norm, Vec2* uv)  // Added UV parameter
{
    MeshSlot_t* m = Mesh_Get(mesh_id);
    if (!m || m->type != 2) {
        *pos = Vec3_Zero();
        *norm = MakeVec3(0, 1, 0);
        if (uv) *uv = MakeVec2(0, 0);
        return;
    }

    if (frame_a >= m->anim.frame_count) frame_a = m->anim.frame_count - 1;
    if (frame_b >= m->anim.frame_count) frame_b = m->anim.frame_count - 1;
    if (vert_idx >= m->anim.verts_per_frame) vert_idx = 0;

    MD2FrameDesc_t* fa = &g_frame_pool[m->anim.frame_start + frame_a];
    MD2FrameDesc_t* fb = &g_frame_pool[m->anim.frame_start + frame_b];

    MD2Vertex_t* va = &g_md2_vertex_pool[fa->vertex_start + vert_idx];
    MD2Vertex_t* vb = &g_md2_vertex_pool[fb->vertex_start + vert_idx];

    Vec3 pa = MakeVec3(
        fa->scale.x * va->x + fa->translate.x,
        fa->scale.y * va->y + fa->translate.y,
        fa->scale.z * va->z + fa->translate.z
    );
    Vec3 pb = MakeVec3(
        fb->scale.x * vb->x + fb->translate.x,
        fb->scale.y * vb->y + fb->translate.y,
        fb->scale.z * vb->z + fb->translate.z
    );
    *pos = Vec3_Lerp(pa, pb, t);

    int ni_a = va->normal_index % 162;
    int ni_b = vb->normal_index % 162;
    Vec3 na = MakeVec3(g_normals[ni_a][0], g_normals[ni_a][1], g_normals[ni_a][2]);
    Vec3 nb = MakeVec3(g_normals[ni_b][0], g_normals[ni_b][1], g_normals[ni_b][2]);
    *norm = Vec3_Normalize(Vec3_Lerp(na, nb, t));

    // Return UV if requested
    if (uv) {
        *uv = MakeVec2(0, 0); // Will be set from triangle data
    }
}
typedef struct { const char* name; int start, end; } MD2Anim_t;

static const MD2Anim_t g_anims[] = {
    {"stand", 0, 39}, {"run", 40, 45}, {"attack", 46, 53},
    {"pain1", 54, 57}, {"pain2", 58, 61}, {"pain3", 62, 65},
    {"jump", 66, 71}, {"flip", 72, 83}, {"salute", 84, 94},
    {"taunt", 95, 111}, {"wave", 112, 122}, {"point", 123, 134},
    {"death1", 178, 183}, {"death2", 184, 189}, {"death3", 190, 197}
};

int MD2_GetAnimRange(const char* name, int* start, int* end)
{
    for (int i = 0; i < (int)(sizeof(g_anims) / sizeof(g_anims[0])); i++) {
        if (strcmp(g_anims[i].name, name) == 0) {
            *start = g_anims[i].start;
            *end = g_anims[i].end;
            return 1;
        }
    }
    return 0;
}