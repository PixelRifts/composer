/* date = September 10th 2024 6:43 pm */

#ifndef QUI_SIMPLE_RENDER_H
#define QUI_SIMPLE_RENDER_H

#include "defines.h"
#include "base/vmath.h"
#include "base/mem.h"

#include "ui.h"

//~ Fo2nt

typedef struct Rift_UIFontGlyph {
    int   id;
    float advance;
    float atlas_left;
    float atlas_bottom;
    float atlas_right;
    float atlas_top;
    float left;
    float bottom;
    float right;
    float top;
} Rift_UIFontGlyph;

typedef struct Rift_UIFont {
    u32 texture;
    float texture_width;
    float texture_height;
    
    Rift_UIFontGlyph* glyphs;
    int glyph_count;
    int glyph_cap;
} Rift_UIFont;

//~ Drawing

typedef struct Rift_UISimpleVertex {
    vec4 pos_size;
    vec2 uv;
    vec4 color;
    vec3 rounding_params;
    vec2 tex_params;
    
    /*
            union {
                struct {
                    float rounding;
                    float softness;
                    float edge;
                };
                float rounding_params[3];
            };
            union {
                struct {
                    float tex_id;
                    float is_sdf;
                };
                float tex_params[2];
            };
            */
} Rift_UISimpleVertex;

#define QUI_MAX_QUADS     2048
#define QUI_MAX_TRIANGLES QUI_MAX_QUADS * 2
#define QUI_MAX_VERTICES  QUI_MAX_TRIANGLES * 3


typedef struct Rift_UISimpleRenderer {
    M_Arena arena;
    
    u32 vao;
    u32 vbo;
    u32 shader;
    u32 textures[16];
    u64 texture_count;
    
    Rift_UISimpleVertex vertices[QUI_MAX_VERTICES];
    u64 triangle_count;
    
    f32 matrix[4*4];
    
    Rift_UIFont fonts[8];
    u32 font_count;
} Rift_UISimpleRenderer;

Rift_UISimpleRenderer* Rift_UIRendererInit(M_Arena* arena, f32 window_width, f32 window_height);
void Rift_UIRendererResize(Rift_UISimpleRenderer* renderer, f32 window_width, f32 window_height);
void Rift_UIRendererBegin(Rift_UISimpleRenderer* renderer);
void Rift_UIRendererDrawQuad(Rift_UISimpleRenderer* renderer,
                             Rift_UISimpleVertex p1, Rift_UISimpleVertex p2,
                             Rift_UISimpleVertex p3, Rift_UISimpleVertex p4,
                             u32 texture);
void Rift_UIRendererDrawBox(Rift_UISimpleRenderer* renderer, Rift_UIBox* box);
void Rift_UIRendererEnd(Rift_UISimpleRenderer* renderer);
void Rift_UIRendererFree(Rift_UISimpleRenderer* renderer);

u32 Rift_UIFontLoad(Rift_UISimpleRenderer* renderer, char* name);

#endif //QUI_SIMPLE_RENDER_H