/**
 * @file resource.h
 * @brief Resource Management System
 * 
 * This file ties together meshes, textures, materials, and shows
 * how they connect to entities.
 * 
 * WORKFLOW:
 * =========
 * 1. Load mesh:     uint32_t mesh_id = Resource_LoadMesh("model.obj");
 * 2. Load texture:  uint32_t tex_id = Resource_LoadTexture("skin.bmp");
 * 3. Create material with texture: uint32_t mat_id = Resource_CreateMaterial(tex_id);
 * 4. Create entity: EntityID ent = Entity_Create("Player");
 * 5. Add renderer:  Entity_AddComponent(ent, COMP_MESH_RENDERER);
 * 6. Assign mesh:   Entity_SetMesh(ent, mesh_id);
 * 7. Assign material: Entity_SetMaterial(ent, mat_id);
 * 
 * That's it! The entity will now be rendered with that mesh and texture.
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Resource IDs
 * 
 * All resources are identified by uint32_t IDs.
 * INVALID_RESOURCE (0xFFFFFFFF) means "not loaded" or "error"
 * ============================================================ */

#define INVALID_RESOURCE 0xFFFFFFFF

typedef uint32_t MeshID;
typedef uint32_t TextureID;
typedef uint32_t MaterialID;

/* ============================================================
 * Resource Manager - Initialize/Shutdown
 * ============================================================ */

void Resource_Init(void);
void Resource_Shutdown(void);

/* ============================================================
 * MESH LOADING
 * 
 * Supported formats:
 * - OBJ (static meshes with UVs and normals)
 * - MD2 (animated Quake 2 models)
 * 
 * You can also create primitives programmatically.
 * ============================================================ */

/* Load from memory (for embedded data) */
MeshID Resource_LoadMeshOBJ(const void* data, uint32_t size, const char* name);
MeshID Resource_LoadMeshMD2(const void* data, uint32_t size, const char* name);

/* Create primitive meshes */
MeshID Resource_CreateCube(float size, const char* name);
MeshID Resource_CreateSphere(float radius, int segments, const char* name);
MeshID Resource_CreatePlane(float width, float height, const char* name);
MeshID Resource_CreateCylinder(float radius, float height, int segments, const char* name);

/* Get mesh by name */
MeshID Resource_FindMesh(const char* name);

/* Free mesh */
void Resource_FreeMesh(MeshID id);

/* ============================================================
 * TEXTURE LOADING
 * 
 * Supported formats:
 * - BMP (24-bit and 8-bit paletted)
 * 
 * Textures are converted to RGB565 format internally.
 * IMPORTANT: Use power-of-2 sizes (64, 128, 256) for best performance!
 * ============================================================ */

/* Load from memory */
TextureID Resource_LoadTextureBMP(const void* data, uint32_t size, const char* name);

/* Create procedural textures */
TextureID Resource_CreateSolidTexture(uint16_t color, int size, const char* name);
TextureID Resource_CreateCheckerTexture(uint16_t c1, uint16_t c2, int size, const char* name);

/* Get texture by name */
TextureID Resource_FindTexture(const char* name);

/* Free texture */
void Resource_FreeTexture(TextureID id);

/* ============================================================
 * MATERIALS
 * 
 * A material combines:
 * - Diffuse texture (the main texture)
 * - Diffuse color (tint, or solid color if no texture)
 * - Rendering flags (unlit, transparent, etc.)
 * ============================================================ */

/* Material flags */
#define MAT_UNLIT       (1 << 0)    /* No lighting calculations */
#define MAT_TRANSPARENT (1 << 1)    /* Enable alpha blending */
#define MAT_DOUBLESIDED (1 << 2)    /* Disable backface culling */

/* Create material */
MaterialID Resource_CreateMaterial(const char* name);

/* Set material properties */
void Material_SetTexture(MaterialID mat, TextureID tex);
void Material_SetColor(MaterialID mat, uint16_t color);
void Material_SetFlags(MaterialID mat, uint32_t flags);

/* Get material by name */
MaterialID Resource_FindMaterial(const char* name);

/* Free material */
void Resource_FreeMaterial(MaterialID id);

/* ============================================================
 * ENTITY INTEGRATION
 * 
 * These functions connect resources to entities.
 * Call these AFTER Entity_AddComponent(ent, COMP_MESH_RENDERER)
 * ============================================================ */

/* Assign mesh to entity */
void Entity_SetMesh(uint32_t entity_id, MeshID mesh);

/* Assign material to entity */  
void Entity_SetMaterial(uint32_t entity_id, MaterialID material);

/* Convenience: Set both mesh and material */
void Entity_SetRenderable(uint32_t entity_id, MeshID mesh, MaterialID material);

/* ============================================================
 * RESOURCE INFO
 * ============================================================ */

/* Get memory usage stats */
uint32_t Resource_GetMeshMemoryUsed(void);
uint32_t Resource_GetTextureMemoryUsed(void);
uint32_t Resource_GetTotalMemoryUsed(void);

/* Get counts */
uint32_t Resource_GetMeshCount(void);
uint32_t Resource_GetTextureCount(void);
uint32_t Resource_GetMaterialCount(void);

#ifdef __cplusplus
}
#endif

#endif /* RESOURCE_H */
