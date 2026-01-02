/**
 * @file resource.cpp
 * @brief Resource Management System Implementation
 */

#include "resource.h"
#include "mesh.h"
#include "texture.h"
#include "entity.h"
#include "platform.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

 /* ============================================================
  * Internal Structures
  * ============================================================ */

#define MAX_RESOURCES 64
#define NAME_LENGTH 32

typedef struct {
    char name[NAME_LENGTH];
    uint8_t type;
    uint8_t flags;
    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t frame_count;
    void* vertex_data;
    uint16_t* index_data;
    Vec3 bounds_center;
    float bounds_radius;
} MeshResource_t;

typedef struct {
    char name[NAME_LENGTH];
    uint8_t in_use;
    uint16_t width;
    uint16_t height;
    uint16_t* pixels;
} TextureResource_t;

typedef struct {
    char name[NAME_LENGTH];
    uint8_t in_use;
    uint32_t flags;
    TextureID texture;
    uint16_t color;
} MaterialResource_t;

/* Resource pools */
static MeshResource_t g_res_meshes[MAX_RESOURCES];
static TextureResource_t g_res_textures[MAX_RESOURCES];
static MaterialResource_t g_res_materials[MAX_RESOURCES];

static uint32_t g_mesh_memory = 0;
static uint32_t g_texture_memory = 0;

/* Helper to create vertex */
static inline Vertex_t MakeVertex(Vec3 pos, Vec3 norm, Vec2 uv)
{
    Vertex_t v;
    v.position = pos;
    v.normal = norm;
    v.texcoord = uv;
    return v;
}

/* ============================================================
 * Initialization
 * ============================================================ */

void Resource_Init(void)
{
    memset(g_res_meshes, 0, sizeof(g_res_meshes));
    memset(g_res_textures, 0, sizeof(g_res_textures));
    memset(g_res_materials, 0, sizeof(g_res_materials));
    g_mesh_memory = 0;
    g_texture_memory = 0;
}

void Resource_Shutdown(void)
{
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (g_res_meshes[i].type != 0) {
            free(g_res_meshes[i].vertex_data);
            free(g_res_meshes[i].index_data);
            g_res_meshes[i].type = 0;
        }
    }

    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (g_res_textures[i].in_use) {
            free(g_res_textures[i].pixels);
            g_res_textures[i].in_use = 0;
        }
    }

    memset(g_res_materials, 0, sizeof(g_res_materials));
    g_mesh_memory = 0;
    g_texture_memory = 0;
}

/* ============================================================
 * Mesh Management
 * ============================================================ */

static MeshID AllocMesh(const char* name)
{
    for (uint32_t i = 0; i < MAX_RESOURCES; i++) {
        if (g_res_meshes[i].type == 0) {
            memset(&g_res_meshes[i], 0, sizeof(MeshResource_t));
            if (name) strncpy(g_res_meshes[i].name, name, NAME_LENGTH - 1);
            return i;
        }
    }
    return INVALID_RESOURCE;
}

MeshID Resource_FindMesh(const char* name)
{
    if (!name) return INVALID_RESOURCE;
    for (uint32_t i = 0; i < MAX_RESOURCES; i++) {
        if (g_res_meshes[i].type != 0 && strcmp(g_res_meshes[i].name, name) == 0) {
            return i;
        }
    }
    return INVALID_RESOURCE;
}

void Resource_FreeMesh(MeshID id)
{
    if (id >= MAX_RESOURCES || g_res_meshes[id].type == 0) return;

    uint32_t mem = g_res_meshes[id].vertex_count * sizeof(Vertex_t) +
        g_res_meshes[id].index_count * sizeof(uint16_t);
    g_mesh_memory -= mem;

    free(g_res_meshes[id].vertex_data);
    free(g_res_meshes[id].index_data);
    memset(&g_res_meshes[id], 0, sizeof(MeshResource_t));
}

/* ============================================================
 * Primitive Mesh Creation
 * ============================================================ */

MeshID Resource_CreateCube(float size, const char* name)
{
    MeshID id = AllocMesh(name);
    if (id == INVALID_RESOURCE) return INVALID_RESOURCE;

    float h = size * 0.5f;

    g_res_meshes[id].vertex_count = 24;
    g_res_meshes[id].index_count = 36;
    g_res_meshes[id].vertex_data = malloc(24 * sizeof(Vertex_t));
    g_res_meshes[id].index_data = (uint16_t*)malloc(36 * sizeof(uint16_t));

    if (!g_res_meshes[id].vertex_data || !g_res_meshes[id].index_data) {
        free(g_res_meshes[id].vertex_data);
        free(g_res_meshes[id].index_data);
        g_res_meshes[id].type = 0;
        return INVALID_RESOURCE;
    }

    Vertex_t* v = (Vertex_t*)g_res_meshes[id].vertex_data;

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

    uint16_t indices[36] = {
        0,1,2,0,2,3, 4,5,6,4,6,7, 8,9,10,8,10,11,
        12,13,14,12,14,15, 16,17,18,16,18,19, 20,21,22,20,22,23
    };
    memcpy(g_res_meshes[id].index_data, indices, sizeof(indices));

    g_res_meshes[id].type = 1;
    g_res_meshes[id].bounds_center = MakeVec3(0, 0, 0);
    g_res_meshes[id].bounds_radius = h * 1.732f;

    g_mesh_memory += 24 * sizeof(Vertex_t) + 36 * sizeof(uint16_t);

    return id;
}

MeshID Resource_CreatePlane(float width, float height, const char* name)
{
    MeshID id = AllocMesh(name);
    if (id == INVALID_RESOURCE) return INVALID_RESOURCE;

    float hw = width * 0.5f, hh = height * 0.5f;

    g_res_meshes[id].vertex_count = 4;
    g_res_meshes[id].index_count = 6;
    g_res_meshes[id].vertex_data = malloc(4 * sizeof(Vertex_t));
    g_res_meshes[id].index_data = (uint16_t*)malloc(6 * sizeof(uint16_t));

    if (!g_res_meshes[id].vertex_data || !g_res_meshes[id].index_data) {
        free(g_res_meshes[id].vertex_data);
        free(g_res_meshes[id].index_data);
        g_res_meshes[id].type = 0;
        return INVALID_RESOURCE;
    }

    Vertex_t* v = (Vertex_t*)g_res_meshes[id].vertex_data;
    v[0] = MakeVertex(MakeVec3(-hw, 0, -hh), MakeVec3(0, 1, 0), MakeVec2(0, 0));
    v[1] = MakeVertex(MakeVec3(hw, 0, -hh), MakeVec3(0, 1, 0), MakeVec2(1, 0));
    v[2] = MakeVertex(MakeVec3(hw, 0, hh), MakeVec3(0, 1, 0), MakeVec2(1, 1));
    v[3] = MakeVertex(MakeVec3(-hw, 0, hh), MakeVec3(0, 1, 0), MakeVec2(0, 1));

    uint16_t* idx = g_res_meshes[id].index_data;
    idx[0] = 0; idx[1] = 2; idx[2] = 1;
    idx[3] = 0; idx[4] = 3; idx[5] = 2;

    g_res_meshes[id].type = 1;
    g_res_meshes[id].bounds_center = MakeVec3(0, 0, 0);
    g_res_meshes[id].bounds_radius = (hw > hh) ? hw : hh;

    g_mesh_memory += 4 * sizeof(Vertex_t) + 6 * sizeof(uint16_t);

    return id;
}

MeshID Resource_CreateSphere(float radius, int segments, const char* name)
{
    MeshID id = AllocMesh(name);
    if (id == INVALID_RESOURCE) return INVALID_RESOURCE;

    if (segments < 4) segments = 4;
    if (segments > 32) segments = 32;

    int rings = segments;
    int sectors = segments * 2;

    uint32_t vert_count = (rings + 1) * (sectors + 1);
    uint32_t idx_count = rings * sectors * 6;

    g_res_meshes[id].vertex_count = vert_count;
    g_res_meshes[id].index_count = idx_count;
    g_res_meshes[id].vertex_data = malloc(vert_count * sizeof(Vertex_t));
    g_res_meshes[id].index_data = (uint16_t*)malloc(idx_count * sizeof(uint16_t));

    if (!g_res_meshes[id].vertex_data || !g_res_meshes[id].index_data) {
        free(g_res_meshes[id].vertex_data);
        free(g_res_meshes[id].index_data);
        g_res_meshes[id].type = 0;
        return INVALID_RESOURCE;
    }

    Vertex_t* verts = (Vertex_t*)g_res_meshes[id].vertex_data;
    uint16_t* indices = g_res_meshes[id].index_data;

    int vi = 0;
    for (int r = 0; r <= rings; r++) {
        float phi = 3.14159f * r / rings;
        float sp = sinf(phi), cp = cosf(phi);

        for (int s = 0; s <= sectors; s++) {
            float theta = 2.0f * 3.14159f * s / sectors;
            float st = sinf(theta), ct = cosf(theta);

            Vec3 n = MakeVec3(ct * sp, cp, st * sp);
            verts[vi].position = Vec3_Scale(n, radius);
            verts[vi].normal = n;
            verts[vi].texcoord = MakeVec2((float)s / sectors, (float)r / rings);
            vi++;
        }
    }

    int ii = 0;
    for (int r = 0; r < rings; r++) {
        for (int s = 0; s < sectors; s++) {
            int cur = r * (sectors + 1) + s;
            int next = cur + sectors + 1;

            indices[ii++] = (uint16_t)cur;
            indices[ii++] = (uint16_t)next;
            indices[ii++] = (uint16_t)(cur + 1);
            indices[ii++] = (uint16_t)(cur + 1);
            indices[ii++] = (uint16_t)next;
            indices[ii++] = (uint16_t)(next + 1);
        }
    }

    g_res_meshes[id].type = 1;
    g_res_meshes[id].bounds_center = MakeVec3(0, 0, 0);
    g_res_meshes[id].bounds_radius = radius;

    g_mesh_memory += vert_count * sizeof(Vertex_t) + idx_count * sizeof(uint16_t);

    return id;
}

MeshID Resource_CreateCylinder(float radius, float height, int segments, const char* name)
{
    MeshID id = AllocMesh(name);
    if (id == INVALID_RESOURCE) return INVALID_RESOURCE;

    if (segments < 3) segments = 3;
    if (segments > 32) segments = 32;

    float hh = height * 0.5f;

    uint32_t vert_count = segments * 2 + (segments + 1) * 2;
    uint32_t idx_count = segments * 6 + segments * 3 * 2;

    g_res_meshes[id].vertex_count = vert_count;
    g_res_meshes[id].index_count = idx_count;
    g_res_meshes[id].vertex_data = malloc(vert_count * sizeof(Vertex_t));
    g_res_meshes[id].index_data = (uint16_t*)malloc(idx_count * sizeof(uint16_t));

    if (!g_res_meshes[id].vertex_data || !g_res_meshes[id].index_data) {
        free(g_res_meshes[id].vertex_data);
        free(g_res_meshes[id].index_data);
        g_res_meshes[id].type = 0;
        return INVALID_RESOURCE;
    }

    Vertex_t* v = (Vertex_t*)g_res_meshes[id].vertex_data;
    uint16_t* idx = g_res_meshes[id].index_data;
    int vi = 0, ii = 0;

    /* Side vertices */
    for (int s = 0; s < segments; s++) {
        float theta = 2.0f * 3.14159f * s / segments;
        float c = cosf(theta), sn = sinf(theta);
        float u = (float)s / segments;
        v[vi++] = MakeVertex(MakeVec3(c * radius, -hh, sn * radius), MakeVec3(c, 0, sn), MakeVec2(u, 1));
        v[vi++] = MakeVertex(MakeVec3(c * radius, hh, sn * radius), MakeVec3(c, 0, sn), MakeVec2(u, 0));
    }

    /* Side indices */
    for (int s = 0; s < segments; s++) {
        int i0 = s * 2, i1 = ((s + 1) % segments) * 2;
        idx[ii++] = (uint16_t)i0; idx[ii++] = (uint16_t)(i0 + 1); idx[ii++] = (uint16_t)i1;
        idx[ii++] = (uint16_t)i1; idx[ii++] = (uint16_t)(i0 + 1); idx[ii++] = (uint16_t)(i1 + 1);
    }

    /* Top cap */
    int tc = vi;
    v[vi++] = MakeVertex(MakeVec3(0, hh, 0), MakeVec3(0, 1, 0), MakeVec2(0.5f, 0.5f));
    for (int s = 0; s < segments; s++) {
        float theta = 2.0f * 3.14159f * s / segments;
        float c = cosf(theta), sn = sinf(theta);
        v[vi++] = MakeVertex(MakeVec3(c * radius, hh, sn * radius), MakeVec3(0, 1, 0), MakeVec2(c * 0.5f + 0.5f, sn * 0.5f + 0.5f));
    }
    for (int s = 0; s < segments; s++) {
        idx[ii++] = (uint16_t)tc;
        idx[ii++] = (uint16_t)(tc + 1 + s);
        idx[ii++] = (uint16_t)(tc + 1 + ((s + 1) % segments));
    }

    /* Bottom cap */
    int bc = vi;
    v[vi++] = MakeVertex(MakeVec3(0, -hh, 0), MakeVec3(0, -1, 0), MakeVec2(0.5f, 0.5f));
    for (int s = 0; s < segments; s++) {
        float theta = 2.0f * 3.14159f * s / segments;
        float c = cosf(theta), sn = sinf(theta);
        v[vi++] = MakeVertex(MakeVec3(c * radius, -hh, sn * radius), MakeVec3(0, -1, 0), MakeVec2(c * 0.5f + 0.5f, sn * 0.5f + 0.5f));
    }
    for (int s = 0; s < segments; s++) {
        idx[ii++] = (uint16_t)bc;
        idx[ii++] = (uint16_t)(bc + 1 + ((s + 1) % segments));
        idx[ii++] = (uint16_t)(bc + 1 + s);
    }

    g_res_meshes[id].type = 1;
    g_res_meshes[id].bounds_center = MakeVec3(0, 0, 0);
    g_res_meshes[id].bounds_radius = sqrtf(radius * radius + hh * hh);

    g_mesh_memory += vert_count * sizeof(Vertex_t) + idx_count * sizeof(uint16_t);

    return id;
}

/* ============================================================
 * Texture Management
 * ============================================================ */

static TextureID AllocTexture(const char* name)
{
    for (uint32_t i = 0; i < MAX_RESOURCES; i++) {
        if (!g_res_textures[i].in_use) {
            memset(&g_res_textures[i], 0, sizeof(TextureResource_t));
            if (name) strncpy(g_res_textures[i].name, name, NAME_LENGTH - 1);
            return i;
        }
    }
    return INVALID_RESOURCE;
}

TextureID Resource_FindTexture(const char* name)
{
    if (!name) return INVALID_RESOURCE;
    for (uint32_t i = 0; i < MAX_RESOURCES; i++) {
        if (g_res_textures[i].in_use && strcmp(g_res_textures[i].name, name) == 0) {
            return i;
        }
    }
    return INVALID_RESOURCE;
}

void Resource_FreeTexture(TextureID id)
{
    if (id >= MAX_RESOURCES || !g_res_textures[id].in_use) return;

    g_texture_memory -= g_res_textures[id].width * g_res_textures[id].height * 2;
    free(g_res_textures[id].pixels);
    memset(&g_res_textures[id], 0, sizeof(TextureResource_t));
}

TextureID Resource_CreateSolidTexture(uint16_t color, int size, const char* name)
{
    TextureID id = AllocTexture(name);
    if (id == INVALID_RESOURCE) return INVALID_RESOURCE;

    g_res_textures[id].width = (uint16_t)size;
    g_res_textures[id].height = (uint16_t)size;
    g_res_textures[id].pixels = (uint16_t*)malloc(size * size * sizeof(uint16_t));

    if (!g_res_textures[id].pixels) {
        g_res_textures[id].in_use = 0;
        return INVALID_RESOURCE;
    }

    for (int i = 0; i < size * size; i++) {
        g_res_textures[id].pixels[i] = color;
    }

    g_res_textures[id].in_use = 1;
    g_texture_memory += size * size * 2;

    return id;
}

TextureID Resource_CreateCheckerTexture(uint16_t c1, uint16_t c2, int size, const char* name)
{
    TextureID id = AllocTexture(name);
    if (id == INVALID_RESOURCE) return INVALID_RESOURCE;

    g_res_textures[id].width = (uint16_t)size;
    g_res_textures[id].height = (uint16_t)size;
    g_res_textures[id].pixels = (uint16_t*)malloc(size * size * sizeof(uint16_t));

    if (!g_res_textures[id].pixels) {
        g_res_textures[id].in_use = 0;
        return INVALID_RESOURCE;
    }

    int check = size / 8;
    if (check < 1) check = 1;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int cx = x / check, cy = y / check;
            g_res_textures[id].pixels[y * size + x] = ((cx + cy) & 1) ? c1 : c2;
        }
    }

    g_res_textures[id].in_use = 1;
    g_texture_memory += size * size * 2;

    return id;
}

/* ============================================================
 * Material Management
 * ============================================================ */

MaterialID Resource_CreateMaterial(const char* name)
{
    for (uint32_t i = 0; i < MAX_RESOURCES; i++) {
        if (!g_res_materials[i].in_use) {
            memset(&g_res_materials[i], 0, sizeof(MaterialResource_t));
            if (name) strncpy(g_res_materials[i].name, name, NAME_LENGTH - 1);
            g_res_materials[i].in_use = 1;
            g_res_materials[i].texture = INVALID_RESOURCE;
            g_res_materials[i].color = 0xFFFF;
            return i;
        }
    }
    return INVALID_RESOURCE;
}

void Material_SetTexture(MaterialID mat, TextureID tex)
{
    if (mat < MAX_RESOURCES && g_res_materials[mat].in_use) {
        g_res_materials[mat].texture = tex;
    }
}

void Material_SetColor(MaterialID mat, uint16_t color)
{
    if (mat < MAX_RESOURCES && g_res_materials[mat].in_use) {
        g_res_materials[mat].color = color;
    }
}

void Material_SetFlags(MaterialID mat, uint32_t flags)
{
    if (mat < MAX_RESOURCES && g_res_materials[mat].in_use) {
        g_res_materials[mat].flags = flags;
    }
}

MaterialID Resource_FindMaterial(const char* name)
{
    if (!name) return INVALID_RESOURCE;
    for (uint32_t i = 0; i < MAX_RESOURCES; i++) {
        if (g_res_materials[i].in_use && strcmp(g_res_materials[i].name, name) == 0) {
            return i;
        }
    }
    return INVALID_RESOURCE;
}

void Resource_FreeMaterial(MaterialID id)
{
    if (id < MAX_RESOURCES) {
        memset(&g_res_materials[id], 0, sizeof(MaterialResource_t));
    }
}

/* ============================================================
 * Entity Integration (stub - implement if Entity system exists)
 * ============================================================ */

#if 0
void Entity_SetMesh(uint32_t entity_id, MeshID mesh)
{
    MeshRenderer_t* mr = Entity_GetMeshRenderer(entity_id);
    if (mr && mesh < MAX_RESOURCES && g_res_meshes[mesh].type != 0) {
        mr->mesh_id = mesh;
        mr->bounds_center = g_res_meshes[mesh].bounds_center;
        mr->bounds_radius = g_res_meshes[mesh].bounds_radius;
    }
}

void Entity_SetMaterial(uint32_t entity_id, MaterialID material)
{
    MeshRenderer_t* mr = Entity_GetMeshRenderer(entity_id);
    if (mr && material < MAX_RESOURCES && g_res_materials[material].in_use) {
        mr->material_id = material;
    }
}

void Entity_SetRenderable(uint32_t entity_id, MeshID mesh, MaterialID material)
{
    Entity_SetMesh(entity_id, mesh);
    Entity_SetMaterial(entity_id, material);
}
#endif

/* ============================================================
 * Resource Info
 * ============================================================ */

uint32_t Resource_GetMeshMemoryUsed(void) { return g_mesh_memory; }
uint32_t Resource_GetTextureMemoryUsed(void) { return g_texture_memory; }
uint32_t Resource_GetTotalMemoryUsed(void) { return g_mesh_memory + g_texture_memory; }

uint32_t Resource_GetMeshCount(void)
{
    uint32_t count = 0;
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (g_res_meshes[i].type != 0) count++;
    }
    return count;
}

uint32_t Resource_GetTextureCount(void)
{
    uint32_t count = 0;
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (g_res_textures[i].in_use) count++;
    }
    return count;
}

uint32_t Resource_GetMaterialCount(void)
{
    uint32_t count = 0;
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (g_res_materials[i].in_use) count++;
    }
    return count;
}

/* ============================================================
 * Mesh/Texture Data Access
 * ============================================================ */

Vertex_t* Resource_GetMeshVertices(MeshID id)
{
    if (id < MAX_RESOURCES && g_res_meshes[id].type == 1) {
        return (Vertex_t*)g_res_meshes[id].vertex_data;
    }
    return NULL;
}

uint16_t* Resource_GetMeshIndices(MeshID id)
{
    if (id < MAX_RESOURCES && g_res_meshes[id].type != 0) {
        return g_res_meshes[id].index_data;
    }
    return NULL;
}

uint32_t Resource_GetMeshVertexCount(MeshID id)
{
    if (id < MAX_RESOURCES) return g_res_meshes[id].vertex_count;
    return 0;
}

uint32_t Resource_GetMeshIndexCount(MeshID id)
{
    if (id < MAX_RESOURCES) return g_res_meshes[id].index_count;
    return 0;
}

uint16_t* Resource_GetTexturePixels(TextureID id)
{
    if (id < MAX_RESOURCES && g_res_textures[id].in_use) {
        return g_res_textures[id].pixels;
    }
    return NULL;
}

uint16_t Resource_GetTextureWidth(TextureID id)
{
    if (id < MAX_RESOURCES) return g_res_textures[id].width;
    return 0;
}

uint16_t Resource_GetTextureHeight(TextureID id)
{
    if (id < MAX_RESOURCES) return g_res_textures[id].height;
    return 0;
}

TextureID Material_GetTexture(MaterialID id)
{
    if (id < MAX_RESOURCES && g_res_materials[id].in_use) {
        return g_res_materials[id].texture;
    }
    return INVALID_RESOURCE;
}

uint16_t Material_GetColor(MaterialID id)
{
    if (id < MAX_RESOURCES && g_res_materials[id].in_use) {
        return g_res_materials[id].color;
    }
    return 0xFFFF;
}

uint32_t Material_GetFlags(MaterialID id)
{
    if (id < MAX_RESOURCES && g_res_materials[id].in_use) {
        return g_res_materials[id].flags;
    }
    return 0;
}