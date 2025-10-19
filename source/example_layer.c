#include "example_layer.h"

#include "os/os.h"
#include "os/input.h"
#include "base/base.h"

static Rift_UIBoxStyle g_tile_main_style = (Rift_UIBoxStyle) {
    .color        = (vec4){0.10f, 0.10f, 0.12f, 1.0f},
    .hot_color    = (vec4){0.15f, 0.15f, 0.17f, 1.0f},
    .active_color = (vec4){0.06f, 0.06f, 0.12f, 1.0f},
    
    .border_color        = (vec4){0.20f, 0.40f, 0.52f, 1.0f},
    .border_hot_color    = (vec4){0.20f, 0.40f, 0.52f, 1.0f},
    .border_active_color = (vec4){0.06f, 0.06f, 0.18f, 1.0f},
    
    .text_color = (vec4){0.8f, 0.8f, 0.94f, 1.0f},
    .font = 0,
    .font_size = 80,
    
    .rounding = 10,
    .softness = 0.1,
    .edge_size = 2,
};

typedef struct CSM_Context {
    M_Arena arena;
    Rift_UIContext* ui;
    
    Rift_UIBox* vertical_center;
    Rift_UIBox* main_block;
    Rift_UIBox* rows [3];
    Rift_UIBox* tiles[9];
} CSM_Context;
static CSM_Context g_ctx;

void CSM_Init(Rift_UIContext* ui, Rift_UIBox* content) {
    arena_init(&g_ctx.arena);
    g_ctx.ui = ui;
    
    content->layout_axis = UIAxis_X;
    content->alignment = UIAlign_Middle;
    
    g_ctx.vertical_center = Rift_UIBoxCreate(ui, content,
                                             Rift_UISizeChildrenSum(), Rift_UISizePct(1),
                                             &g_tile_main_style,
                                             UIBoxFlag_Instant);
    g_ctx.vertical_center->layout_axis = UIAxis_Y;
    g_ctx.vertical_center->alignment   = UIAlign_Middle;
    
    g_ctx.main_block = Rift_UIBoxCreate(ui, g_ctx.vertical_center,
                                        Rift_UISizePixels(600), Rift_UISizePixels(600),
                                        &g_tile_main_style,
                                        UIBoxFlag_Instant | UIBoxFlag_DrawBorder);
    g_ctx.main_block->layout_axis = UIAxis_Y;
    for (int i = 0; i < 3; i++) {
        g_ctx.rows[i] = Rift_UIBoxCreate(ui, g_ctx.main_block,
                                         Rift_UISizePct(1), Rift_UISizePct(1.0f/3.0f),
                                         &g_tile_main_style,
                                         UIBoxFlag_Instant);
        g_ctx.rows[i]->layout_axis = UIAxis_X;
        for (int j = 0; j < 3; j++) {
            g_ctx.tiles[i*3+j] = Rift_UIBoxCreate(ui, g_ctx.rows[i],
                                                  Rift_UISizePct(1.0f/3.0f), Rift_UISizePct(1.0f),
                                                  &g_tile_main_style,
                                                  UIBoxFlag_DrawBack | UIBoxFlag_DrawBorder |
                                                  UIBoxFlag_DrawName | UIBoxFlag_Interactive);
            g_ctx.tiles[i*3+j]->text = str_from_format(&g_ctx.arena, "%d", i*3+j);
        }
    }
}

void CSM_Update(f32 delta) {
    for (int i = 0; i < 9; i++) {
        if (Rift_UIBoxSignal(g_ctx.ui, g_ctx.tiles[i]).clicked) {
            // Do Stuff!!
            printf("Hello %d\n", i);
            flush;
        }
    }
}

void CSM_Render() {}

void CSM_Free() {
    
}
