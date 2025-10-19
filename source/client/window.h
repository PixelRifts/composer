/* date = September 20th 2025 11:01 am */

#ifndef WINDOW_H
#define WINDOW_H

#include "defines.h"
#include "base/str.h"

#include "client/ui.h"

// TODO REMOVE THIS
#include "client/simple_ui_render.h"
#include "client/tri_render.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

typedef struct Rift_WindowProps {
    i32 width;
    i32 height;
    string name;
    
    b8 is_fullscreen;
    b8 custom_titlebar;
} Rift_WindowProps;

typedef struct Rift_Window {
    Rift_WindowProps props;
    
    GLFWwindow* ref;
    Rift_UIContext* ui;
    Rift_UISimpleRenderer* ui_renderer;
    Rift_TriRenderer* tri_renderer;
    
    Rift_UIBox* top_container;
    Rift_UIBox* close_button;
    Rift_UIBox* minimize_button;
} Rift_Window;

void Rift_WindowCreate(Rift_Window* window, Rift_WindowProps props);
void Rift_WindowDestroy(Rift_Window* win);
Rift_UIBox* Rift_WindowCustomTitlebar(Rift_Window* win, Rift_UIContext* ctx, Rift_UISimpleRenderer* renderer,
                                      Rift_TriRenderer* trirenderer);
void Rift_WindowUpdateTitlebar(Rift_Window* win, Rift_UIContext* ctx);

b8   Rift_WindowIsOpen(Rift_Window* win);
void Rift_PollEvents();
void Rift_WindowSwapBuffers(Rift_Window* win);
void Rift_WindowTriggerExit(Rift_Window* win);
void Rift_WindowMinimize(Rift_Window* win);
void Rift_WindowRestore(Rift_Window* win);

void Rift_DrawTitlebar(Rift_Window* win);

#endif //WINDOW_H