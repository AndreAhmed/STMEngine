/**
 * @file main.cpp
 * @brief SDL PC Engine Demo - OBJ and MD2 Loading Example
 */

#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "rendering/math3d.h"
#include "rendering/platform.h"
#include "rendering/engine_config.h"
#include "rendering/device.h"
#include "rendering/rasterizer.h"
#include "rendering/mesh.h"
#include "rendering/texture.h"
#include "rendering/entity.h"

 /* MSVC stdio fix for older SDL versions */
#ifdef _MSC_VER
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE* __cdecl __iob_func(void) { return _iob; }
#endif

/* ============================================================
 * Configuration
 * ============================================================ */
const int SCREEN_WIDTH = DISPLAY_WIDTH;
const int SCREEN_HEIGHT = DISPLAY_HEIGHT;

/* ============================================================
 * Globals
 * ============================================================ */
SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
Device* gDevice = NULL;

/* Camera */
Vec3 g_camera_pos;
Vec3 g_camera_rot;
Mat4 g_view_matrix;
Mat4 g_proj_matrix;

/* Entities */
EntityID g_cube_entity = INVALID_ENTITY;
EntityID g_plane_entity = INVALID_ENTITY;
EntityID g_obj_entity = INVALID_ENTITY;
EntityID g_md2_entity = INVALID_ENTITY;
EntityID g_camera_entity = INVALID_ENTITY;

/* Meshes */
uint32_t g_cube_mesh = 0xFFFFFFFF;
uint32_t g_plane_mesh = 0xFFFFFFFF;
uint32_t g_obj_mesh = 0xFFFFFFFF;
uint32_t g_md2_mesh = 0xFFFFFFFF;
uint32_t g_md2_texture = 0xFFFFFFFF;

/* Textures */
uint32_t g_checker_tex = 0xFFFFFFFF;

/* ============================================================
 * File Loading Helper
 * ============================================================ */
static void* LoadFileToMemory(const char* filename, uint32_t* out_size)
{
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Failed to open file: %s\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void* data = malloc(size);
    if (!data) {
        fclose(f);
        return NULL;
    }

    fread(data, 1, size, f);
    fclose(f);

    *out_size = (uint32_t)size;
    return data;
}

/* ============================================================
 * Rendering Functions
 * ============================================================ */

 /* Transform a vertex from object space to screen space */
static int TransformVertex(const Vertex_t* in, const Mat4* model, ScreenVertex_t* out)
{
    /* Model -> World */
    Vec4 world_pos = Mat4_MultiplyVec4(model, MakeVec4(in->position.x, in->position.y, in->position.z, 1.0f));

    /* World -> View */
    Vec4 view_pos = Mat4_MultiplyVec4(&g_view_matrix, world_pos);

    /* Near plane clip - reject if behind camera */
    if (view_pos.z >= -0.1f) return 0;  // Using near plane distance

    /* View -> Clip */
    Vec4 clip_pos = Mat4_MultiplyVec4(&g_proj_matrix, view_pos);

    /* Clip test - check w */
    if (clip_pos.w <= 0.0001f) return 0;

    /* Perspective divide */
    float inv_w = 1.0f / clip_pos.w;
    float ndc_x = clip_pos.x * inv_w;
    float ndc_y = clip_pos.y * inv_w;
    float ndc_z = clip_pos.z * inv_w;

    /* Frustum culling - allow some slack for edge triangles */
    if (ndc_x < -1.5f || ndc_x > 1.5f) return 0;
    if (ndc_y < -1.5f || ndc_y > 1.5f) return 0;
    if (ndc_z < 0.0f || ndc_z > 1.0f) return 0;

    /* NDC to screen */
    float screen_x = (ndc_x * 0.5f + 0.5f) * SCREEN_WIDTH;
    float screen_y = (1.0f - (ndc_y * 0.5f + 0.5f)) * SCREEN_HEIGHT;

    out->x = (int32_t)(screen_x);
    out->y = (int32_t)(screen_y);
    out->z = ndc_z;
    out->w_inv = inv_w;
    out->u = in->texcoord.x;
    out->v = in->texcoord.y;

    /* Simple lighting based on normal.y (top = bright) */
    float light = 0.3f + 0.7f * (in->normal.y * 0.5f + 0.5f);
    int intensity = (int)(light * 31);
    if (intensity > 31) intensity = 31;
    if (intensity < 0) intensity = 0;
    out->color = RGB565(intensity * 8, intensity * 8, intensity * 8);

    return 1;
}

/* Render a static mesh */
static void RenderStaticMesh(uint32_t mesh_id, const Mat4* model_matrix, uint16_t color)
{
    MeshSlot_t* mesh = Mesh_Get(mesh_id);
    if (!mesh || mesh->type != 1) return;

    Vertex_t* verts = Mesh_GetVertexPtr(mesh->stat.vertex_start);
    uint16_t* indices = Mesh_GetIndexPtr(mesh->stat.index_start);

    if (!verts || !indices) return;
    if (mesh->stat.index_count == 0) return;

    /* Draw triangles */
    for (uint32_t i = 0; i < mesh->stat.index_count; i += 3) {
        ScreenVertex_t sv0, sv1, sv2;

        if (!TransformVertex(&verts[indices[i + 0]], model_matrix, &sv0)) continue;
        if (!TransformVertex(&verts[indices[i + 1]], model_matrix, &sv1)) continue;
        if (!TransformVertex(&verts[indices[i + 2]], model_matrix, &sv2)) continue;
        /* Backface culling (cross product of screen edges) */
        int32_t ax = (sv1.x - sv0.x);
        int32_t ay = (sv1.y - sv0.y);
        int32_t bx = (sv2.x - sv0.x);
        int32_t by = (sv2.y - sv0.y);
        int32_t cross = ax * by - ay * bx;
        if (cross <= 0) continue; /* Cull back-facing
        /* Override vertex colors with mesh color */
        sv0.color = sv1.color = sv2.color = color;

        Rasterizer_DrawTriangleSolid(&sv0, &sv1, &sv2, color);
    }
}

static void RenderMD2Mesh(uint32_t mesh_id, const Mat4* model_matrix,
    uint16_t frame_a, uint16_t frame_b, float lerp, TextureSlot_t* texture)
{
    MeshSlot_t* mesh = Mesh_Get(mesh_id);
    if (!mesh || mesh->type != 2) return;

    uint16_t* indices = Mesh_GetIndexPtr(mesh->anim.index_start);
    if (!indices) return;

    MD2UV_t* uvs = &g_md2_uv_pool[mesh->anim.uv_start];

    /* Draw triangles */
    for (uint32_t i = 0; i < mesh->anim.index_count; i += 3) {
        ScreenVertex_t sv[3];
        int valid = 1;

        for (int j = 0; j < 3; j++) {
            Vec3 pos, norm;
            Vec2 uv_dummy;
            Mesh_GetMD2Vertex(mesh_id, indices[i + j], frame_a, frame_b, lerp, &pos, &norm, &uv_dummy);

            // Get UV from expanded per-triangle storage
            Vec2 uv = MakeVec2(uvs[i + j].u, uvs[i + j].v);

            Vertex_t v;
            v.position = pos;
            v.normal = norm;
            v.texcoord = uv;

            if (!TransformVertex(&v, model_matrix, &sv[j])) {
                valid = 0;
                break;
            }
        }

        if (!valid) continue;

        /* Backface culling */
        int32_t ax = sv[1].x - sv[0].x;
        int32_t ay = sv[1].y - sv[0].y;
        int32_t bx = sv[2].x - sv[0].x;
        int32_t by = sv[2].y - sv[0].y;
        int32_t cross = ax * by - ay * bx;
        if (cross <= 0) continue;

        if (texture) {
            // Convert TextureSlot_t to Texture_t for rasterizer
            Texture_t tex;
            tex.width = texture->width;
            tex.height = texture->height;
            tex.width_mask = texture->width_mask;
            tex.height_mask = texture->height_mask;
            tex.pixels = &g_pixel_pool[texture->pixel_start];

            Rasterizer_DrawTriangle(&sv[0], &sv[1], &sv[2], &tex);
        }
        else {
            Rasterizer_DrawTriangleSolid(&sv[0], &sv[1], &sv[2], COLOR_BLUE);
        }
    }
}
/* ============================================================
 * Update Camera
 * ============================================================ */
static void UpdateCamera(void)
{
    /* Build view matrix (inverse of camera transform) */
    Mat4 rot_x, rot_y, trans, tmp;
    Mat4_RotationX(&rot_x, -g_camera_rot.x);
    Mat4_RotationY(&rot_y, -g_camera_rot.y);
    Mat4_Translation(&trans, -g_camera_pos.x, -g_camera_pos.y, -g_camera_pos.z);

    Mat4_Multiply(&tmp, &rot_x, &rot_y);
    Mat4_Multiply(&g_view_matrix, &tmp, &trans);

    /* Build perspective projection matrix */
    float fov = 60.0f * DEG_TO_RAD;
    float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    Mat4_Perspective(&g_proj_matrix, fov, aspect, near_plane, far_plane);
}

/* ============================================================
 * Initialization
 * ============================================================ */
static bool Init(void)
{
    /* Initialize camera position */
    g_camera_pos = MakeVec3(0.0f, 2.0f, 8.0f);
    g_camera_rot = MakeVec3(0.0f, 0.0f, 0.0f);

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return false;
    }

    /* Create window */
    gWindow = SDL_CreateWindow("3D Engine - OBJ/MD2 Demo",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (!gWindow) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return false;
    }

    /* Get surface and create device */
    gScreenSurface = SDL_GetWindowSurface(gWindow);
    gDevice = new Device(gScreenSurface);

    /* Initialize subsystems */
    Rasterizer_Init();
    Rasterizer_SetDevice(gDevice);
    Mesh_Init();
    Texture_Init();
    Entity_Init();

    /* Create built-in meshes */
    g_cube_mesh = Mesh_CreateCube(1.0f);
    g_plane_mesh = Mesh_CreatePlane(10.0f, 10.0f);

    printf("Created cube mesh: %u\n", g_cube_mesh);
    printf("Created plane mesh: %u\n", g_plane_mesh);

    /* Create checkerboard texture */
    g_checker_tex = Texture_CreateCheckerboard(0xFFFF, 0x8410, 64);
    printf("Created checker texture: %u\n", g_checker_tex);

    /* Try to load OBJ model */
    uint32_t obj_size = 0;
    void* obj_data = LoadFileToMemory("data/suzanne.obj", &obj_size);
    if (obj_data) {
        g_obj_mesh = Mesh_LoadOBJ(obj_data, obj_size);
        free(obj_data);
        printf("Loaded OBJ mesh: %u\n", g_obj_mesh);
    }
    else {
        printf("Note: models/teapot.obj not found, using cube instead\n");
        g_obj_mesh = g_cube_mesh;
    }

    /* Try to load MD2 model */
    uint32_t md2_size = 0;
    void* md2_data = LoadFileToMemory("data/md2/q2mdl-wham/tris.MD2", &md2_size);
    if (md2_data) {
        g_md2_mesh = Mesh_LoadMD2(md2_data, md2_size);
        free(md2_data);
        printf("Loaded MD2 mesh: %u\n", g_md2_mesh);
    }
    else {
        printf("Note: models/player.md2 not found\n");
    }
    uint32_t tex_size = 0;
    void* tex_data = LoadFileToMemory("data/md2/q2mdl-wham/ctf_r.bmp", &tex_size);
    if (tex_data) {
        g_md2_texture = Texture_LoadBMP(tex_data, tex_size);
        free(tex_data);
        printf("Loaded MD2 texture: %u\n", g_md2_texture);
    }
    else {
        printf("Note: models/player.pcx not found, using checkerboard\n");
        g_md2_texture = g_checker_tex;
    }

    /* ============================================================
     * Create Entities
     * ============================================================ */

     /* Camera entity */
    g_camera_entity = Entity_Create("MainCamera");
    Entity_AddComponent(g_camera_entity, COMP_CAMERA);
    Transform_SetPosition(g_camera_entity, g_camera_pos);
    Camera_t* cam = Entity_GetCamera(g_camera_entity);
    if (cam) {
        cam->fov = 60.0f * DEG_TO_RAD;
        cam->near_plane = 0.1f;
        cam->far_plane = 100.0f;
        cam->is_active = 1;
    }

    /* Ground plane */
    g_plane_entity = Entity_Create("Ground");
    Entity_AddComponent(g_plane_entity, COMP_MESH_RENDERER);
    Transform_SetPosition(g_plane_entity, MakeVec3(0, -1, 0));
    MeshRenderer_t* plane_mr = Entity_GetMeshRenderer(g_plane_entity);
    if (plane_mr) {
        plane_mr->mesh_id = g_plane_mesh;
        plane_mr->visible = 1;
        plane_mr->is_animated = 0;
    }

    /* Spinning cube */
    g_cube_entity = Entity_Create("SpinningCube");
    Entity_AddComponent(g_cube_entity, COMP_MESH_RENDERER);
    Transform_SetPosition(g_cube_entity, MakeVec3(-3, 0, 0));
    MeshRenderer_t* cube_mr = Entity_GetMeshRenderer(g_cube_entity);
    if (cube_mr) {
        cube_mr->mesh_id = g_cube_mesh;
        cube_mr->visible = 1;
        cube_mr->is_animated = 0;
    }

    /* OBJ model entity */
    g_obj_entity = Entity_Create("OBJModel");
    Entity_AddComponent(g_obj_entity, COMP_MESH_RENDERER);
    Transform_SetPosition(g_obj_entity, MakeVec3(0, 0, 0));
    Transform_SetScale(g_obj_entity, MakeVec3(0.5f, 0.5f, 0.5f));
    MeshRenderer_t* obj_mr = Entity_GetMeshRenderer(g_obj_entity);
    if (obj_mr) {
        obj_mr->mesh_id = g_obj_mesh;
        obj_mr->visible = 1;
        obj_mr->is_animated = 0;
    }

    /* MD2 animated entity */
    if (g_md2_mesh != 0xFFFFFFFF) {
        g_md2_entity = Entity_Create("MD2Player");
        Entity_AddComponent(g_md2_entity, COMP_MESH_RENDERER);
        Entity_AddComponent(g_md2_entity, COMP_ANIMATOR);
        Transform_SetPosition(g_md2_entity, MakeVec3(3, 0, 0));
        Transform_SetScale(g_md2_entity, MakeVec3(0.05f, 0.05f, 0.05f));
        Transform_SetRotation(g_md2_entity, MakeVec3(-1.8f,  4.9f, 0));

        MeshRenderer_t* md2_mr = Entity_GetMeshRenderer(g_md2_entity);
        if (md2_mr) {
            md2_mr->mesh_id = g_md2_mesh;
            md2_mr->visible = 1;
            md2_mr->is_animated = 1;
            md2_mr->anim_frame_a = 0;
            md2_mr->anim_frame_b = 1;
            md2_mr->anim_lerp = 0;
        }

        /* Set up "stand" animation */
        Animator_t* anim = Entity_GetAnimator(g_md2_entity);
        if (anim) {
            int start, end;
            if (MD2_GetAnimRange("death1", &start, &end)) {
                anim->start_frame = start;
                anim->end_frame = end;
                anim->current_frame = start;
                anim->next_frame = start + 1;
            }
            else {
                anim->start_frame = 0;
                anim->end_frame = 39;
                anim->current_frame = 0;
                anim->next_frame = 1;
            }
            anim->is_playing = 1;
            anim->is_looping = 1;
            anim->playback_speed = 1.0f;
        }
    }

    printf("\nEngine initialized successfully!\n");
    printf("Controls:\n");
    printf("  WASD - Move camera\n");
    printf("  Arrow keys - Rotate camera\n");
    printf("  Space/Ctrl - Move up/down\n");
    printf("  ESC - Quit\n\n");

    return true;
}

/* ============================================================
 * Cleanup
 * ============================================================ */
static void Shutdown(void)
{
    Entity_Shutdown();

    if (gDevice) {
        delete gDevice;
        gDevice = NULL;
    }

    if (gWindow) {
        SDL_DestroyWindow(gWindow);
        gWindow = NULL;
    }

    SDL_Quit();
}

/* ============================================================
 * Main Loop
 * ============================================================ */
int main(int argc, char* args[])
{
    (void)argc; (void)args;

    if (!Init()) {
        printf("Initialization failed!\n");
        return 1;
    }

    bool quit = false;
    SDL_Event e;

    Uint32 last_time = SDL_GetTicks();
    float rotation = 0.0f;

    while (!quit) {
        /* Calculate delta time */
        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        /* Handle events */
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
            }
        }

        /* Handle input */
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        float move_speed = 5.0f * dt;
        float rot_speed = 2.0f * dt;

        /* Camera rotation */
        if (keys[SDL_SCANCODE_LEFT])  g_camera_rot.y -= rot_speed;
        if (keys[SDL_SCANCODE_RIGHT]) g_camera_rot.y += rot_speed;
        if (keys[SDL_SCANCODE_UP])    g_camera_rot.x -= rot_speed;
        if (keys[SDL_SCANCODE_DOWN])  g_camera_rot.x += rot_speed;

        /* Camera movement (relative to view direction) */
        float sin_y = sinf(g_camera_rot.y);
        float cos_y = cosf(g_camera_rot.y);

        if (keys[SDL_SCANCODE_W]) {
            g_camera_pos.x -= sin_y * move_speed;
            g_camera_pos.z -= cos_y * move_speed;
        }
        if (keys[SDL_SCANCODE_S]) {
            g_camera_pos.x += sin_y * move_speed;
            g_camera_pos.z += cos_y * move_speed;
        }
        if (keys[SDL_SCANCODE_A]) {
            g_camera_pos.x -= cos_y * move_speed;
            g_camera_pos.z += sin_y * move_speed;
        }
        if (keys[SDL_SCANCODE_D]) {
            g_camera_pos.x += cos_y * move_speed;
            g_camera_pos.z -= sin_y * move_speed;
        }
        if (keys[SDL_SCANCODE_SPACE]) g_camera_pos.y += move_speed;
        if (keys[SDL_SCANCODE_LCTRL]) g_camera_pos.y -= move_speed;

        /* Update rotation */
        rotation += dt;

        /* Update entity transforms */
        Transform_SetRotation(g_cube_entity, MakeVec3(rotation * 0.5f, rotation, 0));
        Transform_SetRotation(g_obj_entity, MakeVec3(0, rotation * 0.3f, 0));

        if (g_md2_entity != INVALID_ENTITY) {
            //Transform_SetRotation(g_md2_entity, MakeVec3(0, rotation * 0.9f, 0));
        }

        /* Update entity systems */
        Entity_UpdateTransforms();
        Entity_UpdateAnimators(dt);

        /* Sync MD2 animation state */
        if (g_md2_entity != INVALID_ENTITY) {
            Animator_t* anim = Entity_GetAnimator(g_md2_entity);
            MeshRenderer_t* mr = Entity_GetMeshRenderer(g_md2_entity);
            if (anim && mr) {
                mr->anim_frame_a = (uint16_t)anim->current_frame;
                mr->anim_frame_b = (uint16_t)anim->next_frame;
                mr->anim_lerp = anim->interpolation;
            }

        }

        /* Update camera */
        UpdateCamera();

        /* ============================================================
         * Rendering
         * ============================================================ */
        gDevice->Clear(Color(0x20, 0x20, 0x30));
        Rasterizer_ClearDepth();

        /* Iterate through all renderable entities */
        EntityIterator_t it;
        Entity_BeginIteration(&it, COMP_TRANSFORM | COMP_MESH_RENDERER);

        while (Entity_Next(&it)) {
            Transform_t* xform = Entity_GetTransform(it.current);
            MeshRenderer_t* mr = Entity_GetMeshRenderer(it.current);

            if (!xform || !mr || !mr->visible) continue;

            /* Choose color based on entity */
            uint16_t color = 0xFFFF;
            if (it.current == g_cube_entity)  color = COLOR_RED;
            if (it.current == g_plane_entity) color = 0x8410; /* Gray */
            if (it.current == g_obj_entity)   color = COLOR_GREEN;
            if (it.current == g_md2_entity)   color = COLOR_BLUE;

            if (mr->is_animated && mr->mesh_id != 0xFFFFFFFF) {
                /* Render MD2 animated mesh */
                TextureSlot_t* tex = Texture_Get(g_md2_texture);

                RenderMD2Mesh(mr->mesh_id, &xform->world_matrix,
                    mr->anim_frame_a, mr->anim_frame_b, mr->anim_lerp, tex);
            }
            else if (mr->mesh_id != 0xFFFFFFFF) {
                /* Render static mesh */
                RenderStaticMesh(mr->mesh_id, &xform->world_matrix, color);
            }
        }

        /* Present */
        SDL_UpdateWindowSurface(gWindow);

        /* Cap to ~60 FPS */
        Uint32 frame_time = SDL_GetTicks() - current_time;
        if (frame_time < 16) {
            SDL_Delay(16 - frame_time);
        }
    }

    Shutdown();
    return 0;
}