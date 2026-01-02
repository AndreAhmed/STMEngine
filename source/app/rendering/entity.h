/**
 * @file entity.h
 * @brief Complete Entity Component System
 */

#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include "math3d.h"
#include "engine_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t EntityID;
#define INVALID_ENTITY 0xFFFFFFFF

typedef enum {
    COMP_NONE           = 0,
    COMP_TRANSFORM      = (1 << 0),
    COMP_MESH_RENDERER  = (1 << 1),
    COMP_RIGIDBODY      = (1 << 2),
    COMP_CAMERA         = (1 << 3),
    COMP_LIGHT          = (1 << 4),
    COMP_ANIMATOR       = (1 << 5),
    COMP_COLLIDER       = (1 << 6),
    COMP_AUDIO_SOURCE   = (1 << 7)
} ComponentMask;

typedef struct {
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    Mat4 local_matrix;
    Mat4 world_matrix;
    EntityID parent;
    uint8_t dirty;
} Transform_t;

typedef struct {
    uint32_t mesh_id;
    uint32_t material_id;
    Vec3 bounds_center;
    float bounds_radius;
    uint8_t visible;
    uint8_t cast_shadows;
    /* MD2 Animation state */
    uint16_t anim_frame_a;      /* Current frame */
    uint16_t anim_frame_b;      /* Next frame (for interpolation) */
    float anim_lerp;            /* Interpolation factor 0-1 */
    uint8_t is_animated;        /* 1 if this is an MD2 model */
} MeshRenderer_t;

typedef struct {
    float fov;
    float near_plane;
    float far_plane;
    uint8_t is_active;
} Camera_t;

typedef enum { LIGHT_DIRECTIONAL = 0, LIGHT_POINT, LIGHT_SPOT } LightType_t;

typedef struct {
    LightType_t type;
    Vec3 color;
    float intensity;
    float range;
    float spot_angle;
} Light_t;

typedef struct {
    uint32_t current_frame;
    uint32_t next_frame;
    float interpolation;
    float frame_time;
    float playback_speed;
    uint32_t start_frame;
    uint32_t end_frame;
    uint8_t is_playing;
    uint8_t is_looping;
} Animator_t;

typedef struct {
    EntityID id;
    uint32_t components;
    uint8_t active;
    uint8_t layer;
    uint16_t tag;
    char name[24];
} Entity_t;

typedef struct {
    uint32_t index;
    uint32_t required;
    EntityID current;
} EntityIterator_t;

/* Entity API */
void Entity_Init(void);
void Entity_Shutdown(void);
EntityID Entity_Create(const char* name);
void Entity_Destroy(EntityID id);
int Entity_IsValid(EntityID id);
void Entity_SetActive(EntityID id, int active);

void Entity_AddComponent(EntityID id, ComponentMask c);
void Entity_RemoveComponent(EntityID id, ComponentMask c);
int Entity_HasComponent(EntityID id, ComponentMask c);

Transform_t* Entity_GetTransform(EntityID id);
MeshRenderer_t* Entity_GetMeshRenderer(EntityID id);
Camera_t* Entity_GetCamera(EntityID id);
Light_t* Entity_GetLight(EntityID id);
Animator_t* Entity_GetAnimator(EntityID id);

void Entity_SetParent(EntityID child, EntityID parent);
void Transform_SetPosition(EntityID id, Vec3 p);
void Transform_SetRotation(EntityID id, Vec3 r);
void Transform_SetScale(EntityID id, Vec3 s);
Vec3 Transform_GetPosition(EntityID id);
Vec3 Transform_GetForward(EntityID id);
Vec3 Transform_GetRight(EntityID id);
Vec3 Transform_GetUp(EntityID id);

void Entity_UpdateTransforms(void);
void Entity_UpdateAnimators(float dt);
EntityID Entity_FindByName(const char* name);

void Entity_BeginIteration(EntityIterator_t* it, uint32_t req);
int Entity_Next(EntityIterator_t* it);

#ifdef __cplusplus
}
#endif

#endif /* ENTITY_H */
