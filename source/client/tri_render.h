/* date = October 13th 2025 5:04 pm */

#ifndef TRI_RENDER_H
#define TRI_RENDER_H


#include "defines.h"
#include "base/vmath.h"
#include "base/mem.h"

typedef struct Rift_TriVertex {
    vec2 pos;
    vec2 uv;
    vec4 color;
    f32  tex_idx;
} Rift_TriVertex;

#define RTRI_MAX_QUADS     2048
#define RTRI_MAX_TRIANGLES RTRI_MAX_QUADS * 2
#define RTRI_MAX_VERTICES  RTRI_MAX_TRIANGLES * 3

typedef struct Rift_TriRenderer {
    M_Arena arena;
    
    u32 vao;
    u32 vbo;
    u32 shader;
    u32 textures[16];
    u64 texture_count;
    
    Rift_TriVertex vertices[RTRI_MAX_VERTICES];
    u64 triangle_count;
    
    f32 matrix[4*4];
} Rift_TriRenderer;

Rift_TriRenderer* Rift_TriRendererInit(M_Arena* arena, f32 window_width, f32 window_height);
void Rift_TriRendererResize(Rift_TriRenderer* renderer, f32 window_width, f32 window_height);
void Rift_TriRendererBegin(Rift_TriRenderer* renderer);
void Rift_TriRendererDrawTri(Rift_TriRenderer* renderer,
                             Rift_TriVertex p1, Rift_TriVertex p2,
                             Rift_TriVertex p3, u32 texture);
void Rift_TriRendererDrawLine(Rift_TriRenderer* renderer,
                              vec2 start, vec2 end, vec4 color, f32 thickness);
void Rift_TriRendererEnd(Rift_TriRenderer* renderer);
void Rift_TriRendererFree(Rift_TriRenderer* renderer);


#endif //TRI_RENDER_H