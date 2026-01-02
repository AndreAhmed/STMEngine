/**
 * @file loader_obj.cpp
 * @brief Wavefront OBJ File Loader
 */

#include "mesh.h"
#include "platform.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

 /* Temporary parsing buffers */
#define MAX_OBJ_VERTS    8192
#define MAX_OBJ_FACES    16384

typedef struct {
    Vec3* positions;
    Vec3* normals;
    Vec2* texcoords;
    uint32_t pos_count;
    uint32_t norm_count;
    uint32_t tex_count;

    Vertex_t* out_vertices;
    uint16_t* out_indices;
    uint32_t out_vert_count;
    uint32_t out_idx_count;
} OBJParser_t;

static int ParseFloat(const char** str, float* out)
{
    while (**str == ' ' || **str == '\t') (*str)++;
    if (**str == '\0' || **str == '\n') return 0;

    char* end;
    *out = (float)strtod(*str, &end);
    if (end == *str) return 0;
    *str = end;
    return 1;
}

static int ParseInt(const char** str, int* out)
{
    while (**str == ' ' || **str == '\t') (*str)++;
    if (**str == '\0' || **str == '\n') return 0;

    char* end;
    *out = (int)strtol(*str, &end, 10);
    if (end == *str) return 0;
    *str = end;
    return 1;
}

static void ParseFaceIndex(const char** str, int* v, int* vt, int* vn)
{
    *v = *vt = *vn = 0;

    ParseInt(str, v);

    if (**str == '/') {
        (*str)++;
        if (**str != '/') {
            ParseInt(str, vt);
        }
        if (**str == '/') {
            (*str)++;
            ParseInt(str, vn);
        }
    }

    while (**str == ' ' || **str == '\t') (*str)++;
}

static uint32_t FindOrAddVertex(OBJParser_t* p, int vi, int vti, int vni)
{
    vi = (vi > 0) ? vi - 1 : (int)p->pos_count + vi;
    vti = (vti > 0) ? vti - 1 : (vti < 0) ? (int)p->tex_count + vti : -1;
    vni = (vni > 0) ? vni - 1 : (vni < 0) ? (int)p->norm_count + vni : -1;

    Vertex_t vert;
    vert.position = (vi >= 0 && vi < (int)p->pos_count) ? p->positions[vi] : Vec3_Zero();
    vert.texcoord = (vti >= 0 && vti < (int)p->tex_count) ? p->texcoords[vti] : MakeVec2(0, 0);
    vert.normal = (vni >= 0 && vni < (int)p->norm_count) ? p->normals[vni] : MakeVec3(0, 1, 0);

    for (uint32_t i = 0; i < p->out_vert_count; i++) {
        Vertex_t* v = &p->out_vertices[i];
        if (fabsf(v->position.x - vert.position.x) < 0.0001f &&
            fabsf(v->position.y - vert.position.y) < 0.0001f &&
            fabsf(v->position.z - vert.position.z) < 0.0001f &&
            fabsf(v->texcoord.x - vert.texcoord.x) < 0.0001f &&
            fabsf(v->texcoord.y - vert.texcoord.y) < 0.0001f &&
            fabsf(v->normal.x - vert.normal.x) < 0.0001f &&
            fabsf(v->normal.y - vert.normal.y) < 0.0001f &&
            fabsf(v->normal.z - vert.normal.z) < 0.0001f) {
            return i;
        }
    }

    if (p->out_vert_count < MAX_OBJ_VERTS) {
        p->out_vertices[p->out_vert_count] = vert;
        return p->out_vert_count++;
    }

    return 0;
}

/* Use the pool-based loader from mesh.cpp instead */
uint32_t Mesh_LoadOBJ_Memory(const void* data, uint32_t size)
{
    /* Forward to the pool-based implementation */
    return Mesh_LoadOBJ(data, size);
}