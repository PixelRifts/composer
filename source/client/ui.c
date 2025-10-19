#include "ui.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "os/input.h"

//~ Size Related Things

Rift_UISize Rift_UISizePixels(float px) {
    return (Rift_UISize) { .type = UISize_Pixels, .value = px };
}

Rift_UISize Rift_UISizePct(float pct) {
    return (Rift_UISize) { .type = UISize_Percent, .value = pct };
}

Rift_UISize Rift_UISizeTextContent() {
    return (Rift_UISize) { .type = UISize_TextContent };
}

Rift_UISize Rift_UISizeChildrenSum() {
    return (Rift_UISize) { .type = UISize_ChildrenSum };
}

//~ Helpers

static void Rift_UIAutolayout(Rift_UIContext* ctx, Rift_UIBox* root) {
    // Forward recurse pre-order standalone sizing
    U_PtrStackClear(&ctx->layout_stack);
    U_PtrStackPush(&ctx->layout_stack, root);
    
    while (!U_PtrStackIsEmpty(&ctx->layout_stack)) {
        Rift_UIBox* test_box = U_PtrStackPop(&ctx->layout_stack);
        
        // calculate width axis
        switch (test_box->width.type) {
            case UISize_Pixels: {
                test_box->computed_width = test_box->width.value;
            } break;
            
            case UISize_TextContent: {
                // TODO(voxel): 
            } break;
            
            default: break;
        }
        // calculate height axis
        switch (test_box->height.type) {
            case UISize_Pixels: {
                test_box->computed_height = test_box->height.value;
            } break;
            
            case UISize_TextContent: {
                // TODO(voxel): 
            } break;
            
            default: break;
        }
        
        Rift_UIBox* child_iter = test_box->first;
        while (child_iter) {
            U_PtrStackPush(&ctx->layout_stack, child_iter);
            child_iter = child_iter->next;
        }
    }
    
    // Forward recurse post-order dependent sizing
    U_PtrStackClear(&ctx->layout_stack);
    U_PtrStackClear(&ctx->layout_stack_post);
    
    U_PtrStackPush(&ctx->layout_stack, root);
    
    while (!U_PtrStackIsEmpty(&ctx->layout_stack)) {
        Rift_UIBox* curr = U_PtrStackPop(&ctx->layout_stack);
        U_PtrStackPush(&ctx->layout_stack_post, curr);
        
        Rift_UIBox* child_iter = curr->first;
        while (child_iter) {
            U_PtrStackPush(&ctx->layout_stack, child_iter);
            child_iter = child_iter->next;
        }
    }
    
    for (int i = ctx->layout_stack_post.len-1; i >= 0; i--) {
        Rift_UIBox* curr = ctx->layout_stack_post.elems[i];
        
        if (curr->width.type == UISize_ChildrenSum || curr->height.type == UISize_ChildrenSum) {
            float width_max = -100.0f;
            float height_max = -100.0f;
            float width_sum = 0.0f;
            float height_sum = 0.0f;
            Rift_UIBox* child_iter = curr->first;
            while (child_iter) {
                width_sum  += child_iter->computed_width;
                height_sum += child_iter->computed_height;
                width_max  =  Max(width_max,  child_iter->computed_width);
                height_max =  Max(height_max, child_iter->computed_height);
                child_iter = child_iter->next;
            }
            
            if (curr->width.type == UISize_ChildrenSum) {
                curr->computed_width =
                    curr->layout_axis == UIAxis_X ? width_sum : width_max;
            }
            if (curr->height.type == UISize_ChildrenSum) {
                curr->computed_height =
                    curr->layout_axis == UIAxis_Y ? height_sum : height_max;
            }
        }
    }
    
    
    // Forward recurse pre-order dependent sizing
    U_PtrStackClear(&ctx->layout_stack);
    {
        Rift_UIBox* child_iter = root->first;
        while (child_iter) {
            U_PtrStackPush(&ctx->layout_stack, child_iter);
            child_iter = child_iter->next;
        }
    }
    
    while (!U_PtrStackIsEmpty(&ctx->layout_stack)) {
        Rift_UIBox* test_box = U_PtrStackPop(&ctx->layout_stack);
        
        if (test_box->width.type == UISize_Percent) {
            test_box->computed_width =
                test_box->parent->computed_width * test_box->width.value;
            
        } else if (test_box->width.type == UISize_Pixels && test_box->width.value < 0) {
            test_box->computed_width = test_box->parent->computed_width + test_box->width.value;
        }
        if (test_box->height.type == UISize_Percent) {
            test_box->computed_height =
                test_box->parent->computed_height * test_box->height.value;
        } else if (test_box->height.type == UISize_Pixels && test_box->height.value < 0) {
            test_box->computed_height = test_box->parent->computed_height + test_box->height.value;
        }
        
        Rift_UIBox* child_iter = test_box->first;
        while (child_iter) {
            U_PtrStackPush(&ctx->layout_stack, child_iter);
            child_iter = child_iter->next;
        }
    }
    
    // TODO(voxel): Still no idea what solving violations means, putting a
    // TODO(voxel): todo here to remind myself to figure it out.
    
    // Forward recurse pre-order position calculation
    // Merged with absolute position calculation
    U_PtrStackClear(&ctx->layout_stack);
    U_PtrStackPush(&ctx->layout_stack, root);
    
    while (!U_PtrStackIsEmpty(&ctx->layout_stack)) {
        Rift_UIBox* test_box = U_PtrStackPop(&ctx->layout_stack);
        
        float offset = 0.0f;
        
        // Possibly compute actual offset
        if (test_box->alignment != UIAlign_Start) {
            f32 total_childsize = 0.0f;
            Rift_UIBox* child_iter = test_box->first;
            while (child_iter) {
                total_childsize += test_box->layout_axis == UIAxis_X ?
                    child_iter->computed_width : child_iter->computed_height;
                child_iter = child_iter->next;
            }
            
            offset = test_box->computed_width - total_childsize;
            if (test_box->alignment == UIAlign_Middle) offset *= 0.5f;
        }
        
        Rift_UIBox* child_iter = test_box->first;
        while (child_iter) {
            if (test_box->layout_axis == UIAxis_None) {
                child_iter->computed_relative_pos.x = child_iter->pos.x;
                child_iter->computed_relative_pos.y = child_iter->pos.y;
            } else {
                if (test_box->layout_axis == UIAxis_X)
                    child_iter->computed_relative_pos.x = offset;
                else
                    child_iter->computed_relative_pos.y = offset;
                
                offset +=
                    test_box->layout_axis == UIAxis_X ?
                    child_iter->computed_width : child_iter->computed_height;
            }
            
            child_iter = child_iter->next;
        }
        
        test_box->computed_pos.x = test_box->parent
            ? test_box->parent->computed_pos.x + test_box->computed_relative_pos.x : test_box->computed_relative_pos.x;
        
        test_box->computed_pos.y = test_box->parent
            ? test_box->parent->computed_pos.y + test_box->computed_relative_pos.y : test_box->computed_relative_pos.y;
        
        child_iter = test_box->first;
        while (child_iter) {
            U_PtrStackPush(&ctx->layout_stack, child_iter);
            child_iter = child_iter->next;
        }
    }
}

//~ Main Stuff

Rift_UISignal Rift_UIBoxSignal(Rift_UIContext* ctx, Rift_UIBox* box) {
    Rift_UISignal ret = {0};
    
    float mouse_x = OS_InputMouseX();
    float mouse_y = OS_InputMouseY();
    ret.hovering = rect_contains_point((rect) {
                                           box->render_pos.x, box->render_pos.y,
                                           box->render_width, box->render_height
                                       }, v2(mouse_x, mouse_y));
    
    if (box->flags & UIBoxFlag_Interactive) {
        if (ctx->hot != box) return ret;
        
        ret.pressed  = OS_InputMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        ret.released = OS_InputMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT);
        ret.clicked  = box->down_last_frame && ret.released;
        
        if (ret.pressed) {
            box->anchored = box->pos;
        }
        
        if (ctx->last_clicked == box) {
            ret.dragX = OS_InputMouseDX();
            ret.dragY = OS_InputMouseDY();
        }
        
        box->down_last_frame = OS_InputMouseButton(GLFW_MOUSE_BUTTON_LEFT);
    } else
        box->down_last_frame = false;
    
    return ret;
}

Rift_UIBox* Rift_UIBoxCreate(Rift_UIContext* ctx, Rift_UIBox* parent,
                             Rift_UISize width, Rift_UISize height,
                             Rift_UIBoxStyle* style,
                             Rift_UIBoxFlags flags) {
    
    Rift_UIBox* ret = pool_alloc(&ctx->box_allocator);
    MemoryZeroStruct(ret, Rift_UIBox);
    
    ret->flags  = flags;
    ret->width  = width;
    ret->height = height;
    memmove(&ret->style, style, sizeof(Rift_UIBoxStyle));
    ret->initial_interp = true;
    ret->rate = 0.0001f;
    
    ret->parent = parent;
    if (parent->first && parent->last) {
        parent->last->next = ret;
        ret->prev = parent->last;
        parent->last = ret;
    } else {
        parent->first = ret;
        parent->last  = ret;
    }
    
    return ret;
}

void Rift_UIBoxDestroy(Rift_UIContext* ctx, Rift_UIBox* box) {
    if (box->parent) {
        if (box->parent->first == box) box->parent->first = box->next;
        if (box->parent->last  == box) box->parent->last  = box->prev;
        if (box->prev) box->prev->next = box->next;
        if (box->next) box->next->prev = box->prev;
    }
    pool_dealloc(&ctx->box_allocator, box);
}

Rift_UIContext* Rift_UIContextCreate(M_Arena* arena, f32 width, f32 height) {
    Rift_UIContext* ctx = arena_alloc_zero(arena, sizeof(Rift_UIContext));
    
    pool_init(&ctx->box_allocator, sizeof(Rift_UIBox));
    arena_init(&ctx->extra_allocator);
    arena_init(&ctx->frame_allocator[0]);
    arena_init(&ctx->frame_allocator[1]);
    
    ctx->container = arena_alloc_zero(&ctx->extra_allocator, sizeof(Rift_UIBox));
    ctx->container->width  = Rift_UISizePixels(width);
    ctx->container->height = Rift_UISizePixels(height);
    ctx->container->pos.x = 0;
    ctx->container->pos.y = 0;
    
    U_PtrStackInit(&ctx->layout_stack, 32);
    U_PtrStackInit(&ctx->layout_stack_post, 32);
    
    return ctx;
}

void Rift_UIContextUpdate(Rift_UIContext* ctx, f32 delta) {
    Rift_UIAutolayout(ctx, ctx->container);
    
    //~ Hot/Active Tests
    ctx->hot = nullptr;
    ctx->active = nullptr;
    if (!OS_InputMouseButton(GLFW_MOUSE_BUTTON_LEFT))
        ctx->last_clicked = nullptr;
    
    vec2 mouse = OS_InputDirectMousePos();
    U_PtrStackClear(&ctx->layout_stack);
    {
        Rift_UIBox* curr = ctx->container->first;
        while (curr) {
            U_PtrStackPush(&ctx->layout_stack, curr);
            curr = curr->next;
        }
    }
    while (!U_PtrStackIsEmpty(&ctx->layout_stack)) {
        Rift_UIBox* curr = U_PtrStackPop(&ctx->layout_stack);
        
        if (curr->flags & UIBoxFlag_Interactive) {
            if (rect_contains_point((rect) {
                                        curr->computed_pos.x, curr->computed_pos.y,
                                        curr->computed_width, curr->computed_height
                                    }, mouse)) {
                ctx->hot = curr;
                
                if (OS_InputMouseButton(GLFW_MOUSE_BUTTON_LEFT)) {
                    ctx->last_clicked = curr;
                    ctx->active = curr;
                }
            }
        }
        
        Rift_UIBox* child_iter = curr->first;
        while (child_iter) {
            U_PtrStackPush(&ctx->layout_stack, child_iter);
            child_iter = child_iter->next;
        }
    }
    
    
    //~ Interpolation
    
    U_PtrStackClear(&ctx->layout_stack);
    U_PtrStackPush(&ctx->layout_stack, ctx->container);
    
    while (!U_PtrStackIsEmpty(&ctx->layout_stack)) {
        Rift_UIBox* curr = U_PtrStackPop(&ctx->layout_stack);
        b8 instant = !!(curr->flags & UIBoxFlag_Instant) || curr->initial_interp;
        
        curr->initial_interp = false;
        curr->render_pos.x  = animate_f32exp(curr->render_pos.x,  curr->computed_pos.x,  curr->rate, delta, instant);
        curr->render_pos.y  = animate_f32exp(curr->render_pos.y,  curr->computed_pos.y,  curr->rate, delta, instant);
        curr->render_width  = animate_f32exp(curr->render_width,  curr->computed_width,  curr->rate, delta, instant);
        curr->render_height = animate_f32exp(curr->render_height, curr->computed_height, curr->rate, delta, instant);
        
        vec4 picked_color = curr->style.color;
        vec4 picked_border_color = curr->style.border_color;
        if (curr == ctx->active && !(curr->flags & UIBoxFlag_NoActiveAnim)) {
            picked_color = curr->style.active_color;
            picked_border_color = curr->style.border_active_color;
        } else if (curr == ctx->hot && !(curr->flags & UIBoxFlag_NoHotAnim)) {
            picked_color = curr->style.hot_color;
            picked_border_color = curr->style.border_hot_color;
        }
        
        curr->render_style.color        = animate_vec4exp(curr->render_style.color,        picked_color,           curr->rate, delta, instant);
        curr->render_style.border_color = animate_vec4exp(curr->render_style.border_color, picked_border_color,    curr->rate, delta, instant);
        curr->render_style.text_color   = animate_vec4exp(curr->render_style.text_color,   curr->style.text_color, curr->rate, delta, instant);
        curr->render_style.font_size    = animate_f32exp (curr->render_style.font_size,    curr->style.font_size,  curr->rate, delta, instant);
        curr->render_style.rounding     = animate_f32exp (curr->render_style.rounding,     curr->style.rounding,   curr->rate, delta, instant);
        curr->render_style.softness     = animate_f32exp (curr->render_style.softness,     curr->style.softness,   curr->rate, delta, instant);
        curr->render_style.edge_size    = animate_f32exp (curr->render_style.edge_size,    curr->style.edge_size,  curr->rate, delta, instant);
        
        curr->render_style.font = curr->style.font;
        
        Rift_UIBox* child_iter = curr->first;
        while (child_iter) {
            U_PtrStackPush(&ctx->layout_stack, child_iter);
            child_iter = child_iter->next;
        }
    }
}

void Rift_UIResize(Rift_UIContext* ctx, f32 width, f32 height) {
    ctx->container->width  = Rift_UISizePixels(width);
    ctx->container->height = Rift_UISizePixels(height);
}


void Rift_UIContextDestroy(Rift_UIContext* ctx) {
    U_PtrStackFree(&ctx->layout_stack);
    U_PtrStackFree(&ctx->layout_stack_post);
    
    pool_free(&ctx->box_allocator);
    arena_free(&ctx->extra_allocator);
    arena_free(&ctx->frame_allocator[0]);
    arena_free(&ctx->frame_allocator[1]);
}

// REPLACE WITH CUSTOM RENDERER??

void Rift_UIContextDraw(Rift_UIContext* ctx,
                        Rift_UISimpleRenderer* renderer, Rift_TriRenderer* trirenderer) {
    
    U_PtrStackClear(&ctx->layout_stack);
    U_PtrStackPush(&ctx->layout_stack, ctx->container);
    while (!U_PtrStackIsEmpty(&ctx->layout_stack)) {
        Rift_UIBox* curr = U_PtrStackPop(&ctx->layout_stack);
        
        if (!(curr->flags & UIBoxFlag_NoStandardDraw))
            Rift_UIRendererDrawBox(renderer, curr);
        if (curr->flags & UIBoxFlag_CustomDraw)
            curr->render_fn(curr, renderer, trirenderer);
        
        Rift_UIBox* child_iter = curr->first;
        while (child_iter) {
            U_PtrStackPush(&ctx->layout_stack, child_iter);
            child_iter = child_iter->next;
        }
    }
}
