/* date = September 21st 2025 10:09 pm */

#ifndef UI_H
#define UI_H

#include "defines.h"
#include "base/base.h"

typedef enum Rift_UISizeType {
    UISize_None,
    UISize_Pixels,
    UISize_Percent,
    UISize_TextContent,
    UISize_ChildrenSum,
} Rift_UISizeType;

typedef struct Rift_UISize {
    Rift_UISizeType type;
    f32 value;
} Rift_UISize;

Rift_UISize Rift_UISizePixels(float px);
Rift_UISize Rift_UISizePct(float pct);
Rift_UISize Rift_UISizeTextContent();
Rift_UISize Rift_UISizeChildrenSum();

typedef enum Rift_UIAxis {
    UIAxis_None,
    UIAxis_X,
    UIAxis_Y,
} Rift_UIAxis;

typedef enum Rift_UIAlignment {
    UIAlign_Start,
    UIAlign_Middle,
    UIAlign_End,
} Rift_UIAlignment;

typedef struct Rift_UISignal {
    b8 clicked;
	b8 double_clicked; // TODO
	b8 right_clicked;
	b8 pressed;
	b8 released;
	float dragX;
	float dragY;
	b8 hovering;
} Rift_UISignal;


typedef struct Rift_UIBoxStyle {
    vec4 color;
    vec4 hot_color;
    vec4 active_color;
    
    u32  font;
    f32  font_size;
    
    vec4 border_color;
    vec4 border_hot_color;
    vec4 border_active_color;
    
    vec4 text_color;
    
    f32  rounding;
    f32  softness;
    f32  edge_size;
} Rift_UIBoxStyle;


typedef enum Rift_UIBoxFlags {
    UIBoxFlag_None           = 0x0,
    UIBoxFlag_DrawBack       = 0x1,
    UIBoxFlag_DrawBorder     = 0x2,
    UIBoxFlag_Interactive    = 0x4,
    UIBoxFlag_NoHotAnim      = 0x8,
    UIBoxFlag_NoActiveAnim   = 0x10,
    UIBoxFlag_DrawName       = 0x20,
    UIBoxFlag_Instant        = 0x40,
    UIBoxFlag_NoStandardDraw = 0x80,
    UIBoxFlag_CustomDraw     = 0x100,
} Rift_UIBoxFlags;

typedef struct Rift_UIBox Rift_UIBox;

typedef struct Rift_UISimpleRenderer Rift_UISimpleRenderer;
void Rift_UIRendererDrawBox(Rift_UISimpleRenderer* renderer, Rift_UIBox* box);
typedef struct Rift_TriRenderer Rift_TriRenderer;

typedef void Rift_UIBoxCustomRenderFunction(Rift_UIBox* box, Rift_UISimpleRenderer* boxrenderer, Rift_TriRenderer* trirenderer);

struct Rift_UIBox {
    Rift_UIBoxFlags flags;
    
    Rift_UIBox* parent;
    Rift_UIBox* next;
    Rift_UIBox* prev;
    Rift_UIBox* first;
    Rift_UIBox* last;
    
    /*
        struct Rift_UIBox* hash_next;
        struct Rift_UIBox* hash_prev;
        */
    
    // Input stuff
    b8 down_last_frame;
    vec2 anchored;
    
    // Set props
    vec2 pos;
    Rift_UISize  width;
    Rift_UISize  height;
    Rift_UIAxis  layout_axis;
    Rift_UIAlignment alignment;
    Rift_UIBoxStyle  style;
    f32 rate;
    
    // Layout-Computed props
    vec2 computed_relative_pos;
    vec2 computed_pos;
    f32  computed_width;
    f32  computed_height;
    
    // Interpolated props
    b8 initial_interp;
    
    vec2 render_pos;
    f32  render_width;
    f32  render_height;
    Rift_UIBoxStyle render_style;
    
    void* custom_context;
    Rift_UIBoxCustomRenderFunction* render_fn;
    
    string text;
};

typedef struct Rift_UIContext {
    M_Pool  box_allocator;
    M_Arena extra_allocator;
    
    M_Arena frame_allocator[2];
    int current_frame_allocator;
    
    Rift_UIBox* container;
    
    Rift_UIBox* hot;
    Rift_UIBox* active;
    Rift_UIBox* last_clicked;
    
    U_PtrStack layout_stack;
    U_PtrStack layout_stack_post;
} Rift_UIContext;

Rift_UIContext* Rift_UIContextCreate(M_Arena* arena, f32 width, f32 height);
void            Rift_UIContextUpdate(Rift_UIContext* ctx, f32 delta);
void            Rift_UIContextDestroy(Rift_UIContext* ctx);


void Rift_UIResize(Rift_UIContext* ctx, f32 width, f32 height);

Rift_UISignal Rift_UIBoxSignal(Rift_UIContext* ctx, Rift_UIBox* box);
Rift_UIBox* Rift_UIBoxCreate(Rift_UIContext* ctx, Rift_UIBox* parent,
                             Rift_UISize width, Rift_UISize height,
                             Rift_UIBoxStyle* style,
                             Rift_UIBoxFlags flags);
void Rift_UIBoxDestroy(Rift_UIContext* ctx, Rift_UIBox* box);

// MAYBE MODULARIZE

void Rift_UIContextDraw(Rift_UIContext* ctx,
                        Rift_UISimpleRenderer* renderer, Rift_TriRenderer* trirenderer);

#endif //UI_H