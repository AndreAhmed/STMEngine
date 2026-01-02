/**
 * @file entity.c
 * @brief Complete Entity Component System Implementation
 */

#include "entity.h"
#include <string.h>
#include <math.h>

static Entity_t g_entities[MAX_ENTITIES];
static Transform_t g_transforms[MAX_ENTITIES];
static MeshRenderer_t g_mesh_renderers[MAX_ENTITIES];
static Camera_t g_cameras[MAX_ENTITIES];
static Light_t g_lights[MAX_ENTITIES];
static Animator_t g_animators[MAX_ENTITIES];
static uint32_t g_next_id = 1;

static int FindIndex(EntityID id) {
    if (id == INVALID_ENTITY) return -1;
    for (uint32_t i = 0; i < MAX_ENTITIES; i++)
        if (g_entities[i].id == id) return (int)i;
    return -1;
}

void Entity_Init(void)
{
    memset(g_entities, 0, sizeof(g_entities));
    memset(g_transforms, 0, sizeof(g_transforms));
    memset(g_mesh_renderers, 0, sizeof(g_mesh_renderers));
    memset(g_cameras, 0, sizeof(g_cameras));
    memset(g_lights, 0, sizeof(g_lights));
    memset(g_animators, 0, sizeof(g_animators));
    
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        g_entities[i].id = INVALID_ENTITY;
        g_transforms[i].scale = Vec3_One();
        g_transforms[i].parent = INVALID_ENTITY;
        Mat4_Identity(&g_transforms[i].local_matrix);
        Mat4_Identity(&g_transforms[i].world_matrix);
    }
    g_next_id = 1;
}

void Entity_Shutdown(void) { Entity_Init(); }

EntityID Entity_Create(const char* name)
{
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        if (g_entities[i].id == INVALID_ENTITY) {
            EntityID id = g_next_id++;
            g_entities[i].id = id;
            g_entities[i].components = COMP_TRANSFORM;
            g_entities[i].active = 1;
            g_entities[i].layer = 0;
            g_entities[i].tag = 0;
            if (name) strncpy(g_entities[i].name, name, 23);
            
            g_transforms[i].position = Vec3_Zero();
            g_transforms[i].rotation = Vec3_Zero();
            g_transforms[i].scale = Vec3_One();
            g_transforms[i].parent = INVALID_ENTITY;
            g_transforms[i].dirty = 1;
            
            g_mesh_renderers[i].visible = 1;
            g_cameras[i].fov = 60.0f * DEG_TO_RAD;
            g_cameras[i].near_plane = 0.1f;
            g_cameras[i].far_plane = 1000.0f;
            g_lights[i].color = Vec3_One();
            g_lights[i].intensity = 1.0f;
            g_lights[i].range = 10.0f;
            g_animators[i].playback_speed = 1.0f;
            
            return id;
        }
    }
    return INVALID_ENTITY;
}

void Entity_Destroy(EntityID id)
{
    int idx = FindIndex(id);
    if (idx >= 0) {
        for (uint32_t i = 0; i < MAX_ENTITIES; i++)
            if (g_transforms[i].parent == id) {
                g_transforms[i].parent = INVALID_ENTITY;
                g_transforms[i].dirty = 1;
            }
        g_entities[idx].id = INVALID_ENTITY;
        g_entities[idx].components = 0;
    }
}

int Entity_IsValid(EntityID id) { return FindIndex(id) >= 0; }

void Entity_SetActive(EntityID id, int active) {
    int idx = FindIndex(id);
    if (idx >= 0) g_entities[idx].active = active ? 1 : 0;
}

void Entity_AddComponent(EntityID id, ComponentMask c) {
    int idx = FindIndex(id);
    if (idx >= 0) g_entities[idx].components |= c;
}

void Entity_RemoveComponent(EntityID id, ComponentMask c) {
    int idx = FindIndex(id);
    if (idx >= 0) g_entities[idx].components &= ~c;
}

int Entity_HasComponent(EntityID id, ComponentMask c) {
    int idx = FindIndex(id);
    return (idx >= 0) ? (g_entities[idx].components & c) == c : 0;
}

Transform_t* Entity_GetTransform(EntityID id) {
    int idx = FindIndex(id); return (idx >= 0) ? &g_transforms[idx] : NULL;
}
MeshRenderer_t* Entity_GetMeshRenderer(EntityID id) {
    int idx = FindIndex(id); return (idx >= 0) ? &g_mesh_renderers[idx] : NULL;
}
Camera_t* Entity_GetCamera(EntityID id) {
    int idx = FindIndex(id); return (idx >= 0) ? &g_cameras[idx] : NULL;
}
Light_t* Entity_GetLight(EntityID id) {
    int idx = FindIndex(id); return (idx >= 0) ? &g_lights[idx] : NULL;
}
Animator_t* Entity_GetAnimator(EntityID id) {
    int idx = FindIndex(id); return (idx >= 0) ? &g_animators[idx] : NULL;
}

void Entity_SetParent(EntityID child, EntityID parent) {
    int idx = FindIndex(child);
    if (idx >= 0) { g_transforms[idx].parent = parent; g_transforms[idx].dirty = 1; }
}

void Transform_SetPosition(EntityID id, Vec3 p) {
    int idx = FindIndex(id);
    if (idx >= 0) { g_transforms[idx].position = p; g_transforms[idx].dirty = 1; }
}

void Transform_SetRotation(EntityID id, Vec3 r) {
    int idx = FindIndex(id);
    if (idx >= 0) { g_transforms[idx].rotation = r; g_transforms[idx].dirty = 1; }
}

void Transform_SetScale(EntityID id, Vec3 s) {
    int idx = FindIndex(id);
    if (idx >= 0) { g_transforms[idx].scale = s; g_transforms[idx].dirty = 1; }
}

Vec3 Transform_GetPosition(EntityID id) {
    int idx = FindIndex(id);
    return (idx >= 0) ? g_transforms[idx].position : Vec3_Zero();
}

Vec3 Transform_GetForward(EntityID id) {
    int idx = FindIndex(id);
    if (idx < 0) { Vec3 f; f.x = 0; f.y = 0; f.z = -1; return f; }
    Mat4* m = &g_transforms[idx].world_matrix;
    Vec3 forward;
    forward.x = -m->m[8];
    forward.y = -m->m[9];
    forward.z = -m->m[10];
    return Vec3_Normalize(forward);
}

Vec3 Transform_GetRight(EntityID id) {
    int idx = FindIndex(id);
    if (idx < 0) {
        Vec3 right = {1, 0, 0};
        return right;
    }
    Mat4* m = &g_transforms[idx].world_matrix;
    Vec3 right;
    right.x = m->m[0];
    right.y = m->m[1];
    right.z = m->m[2];
    return Vec3_Normalize(right);
}

Vec3 Transform_GetUp(EntityID id) {
    int idx = FindIndex(id);
    if (idx < 0) {
        Vec3 up = {0, 1, 0};
        return up;
    }
    Mat4* m = &g_transforms[idx].world_matrix;
    Vec3 up;
    up.x = m->m[4];
    up.y = m->m[5];
    up.z = m->m[6];
    return Vec3_Normalize(up);
}

static void UpdateLocalMatrix(int idx)
{
    Transform_t* t = &g_transforms[idx];
    Mat4 T, Rx, Ry, Rz, S, tmp;
    
    Mat4_Translation(&T, t->position.x, t->position.y, t->position.z);
    Mat4_RotationX(&Rx, t->rotation.x);
    Mat4_RotationY(&Ry, t->rotation.y);
    Mat4_RotationZ(&Rz, t->rotation.z);
    Mat4_Scale(&S, t->scale.x, t->scale.y, t->scale.z);
    
    Mat4_Multiply(&tmp, &Ry, &Rx);
    Mat4_Multiply(&Rx, &tmp, &Rz);
    Mat4_Multiply(&tmp, &T, &Rx);
    Mat4_Multiply(&t->local_matrix, &tmp, &S);
}

void Entity_UpdateTransforms(void)
{
    /* Update local matrices for dirty transforms */
    for (uint32_t i = 0; i < MAX_ENTITIES; i++)
        if (g_entities[i].id != INVALID_ENTITY && g_transforms[i].dirty)
            UpdateLocalMatrix(i);
    
    /* Propagate world matrices (multiple passes for hierarchy) */
    for (int pass = 0; pass < 8; pass++) {
        int any_dirty = 0;
        for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
            if (g_entities[i].id == INVALID_ENTITY || !g_transforms[i].dirty) continue;
            
            Transform_t* t = &g_transforms[i];
            if (t->parent == INVALID_ENTITY) {
                memcpy(&t->world_matrix, &t->local_matrix, sizeof(Mat4));
                t->dirty = 0;
            } else {
                int pidx = FindIndex(t->parent);
                if (pidx >= 0 && !g_transforms[pidx].dirty) {
                    Mat4_Multiply(&t->world_matrix, &g_transforms[pidx].world_matrix, &t->local_matrix);
                    t->dirty = 0;
                } else {
                    any_dirty = 1;
                }
            }
        }
        if (!any_dirty) break;
    }
}

void Entity_UpdateAnimators(float dt)
{
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        if (g_entities[i].id == INVALID_ENTITY) continue;
        if (!(g_entities[i].components & COMP_ANIMATOR)) continue;
        
        Animator_t* anim = &g_animators[i];
        if (!anim->is_playing) continue;
        
        anim->frame_time += dt * anim->playback_speed;
        
        float frame_duration = 1.0f / 10.0f; /* 10 FPS default */
        if (anim->frame_time >= frame_duration) {
            anim->frame_time -= frame_duration;
            anim->current_frame = anim->next_frame;
            anim->next_frame++;
            
            if (anim->next_frame > anim->end_frame) {
                if (anim->is_looping) {
                    anim->next_frame = anim->start_frame;
                } else {
                    anim->next_frame = anim->end_frame;
                    anim->is_playing = 0;
                }
            }
        }
        
        anim->interpolation = anim->frame_time / frame_duration;
    }
}

EntityID Entity_FindByName(const char* name)
{
    if (!name) return INVALID_ENTITY;
    for (uint32_t i = 0; i < MAX_ENTITIES; i++)
        if (g_entities[i].id != INVALID_ENTITY && strcmp(g_entities[i].name, name) == 0)
            return g_entities[i].id;
    return INVALID_ENTITY;
}

void Entity_BeginIteration(EntityIterator_t* it, uint32_t req) {
    it->index = 0; it->required = req; it->current = INVALID_ENTITY;
}

int Entity_Next(EntityIterator_t* it)
{
    while (it->index < MAX_ENTITIES) {
        uint32_t i = it->index++;
        if (g_entities[i].id != INVALID_ENTITY && g_entities[i].active &&
            (g_entities[i].components & it->required) == it->required) {
            it->current = g_entities[i].id;
            return 1;
        }
    }
    it->current = INVALID_ENTITY;
    return 0;
}
