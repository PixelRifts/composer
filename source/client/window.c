#include "window.h"

#include <glad/glad.h>
#include "os/input.h"


//- Callbacks

void rift_glfw_titlebar_hittest(GLFWwindow* window, int xpos, int ypos, int* hit) {
    Rift_Window* data = glfwGetWindowUserPointer(window);
    
    if (!data->ui) {
        *hit = (ypos <= 20);
    } else {
        *hit = data->ui->hot == data->top_container;
    }
}

void rift_glfw_window_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    OS_InputProject(width, height, width, height);
    
    // TODO MOVE TO CUSTOM CALLBACK
    
    Rift_Window* data = glfwGetWindowUserPointer(window);
    if (data->ui) Rift_UIResize(data->ui, (f32)width, (f32)height);
    if (data->ui_renderer) Rift_UIRendererResize(data->ui_renderer, (f32)width, (f32)height);
    if (data->tri_renderer) Rift_TriRendererResize(data->tri_renderer, (f32)width, (f32)height);
}

void rift_glfw_mouse_event_callback(GLFWwindow* window, int button, int action, int mods) {
    OS_InputButtonCallback(button, action);
}

void rift_glfw_cursor_pos_event_callback(GLFWwindow* window, double xpos, double ypos) {
    OS_InputCursorPosCallback((float)xpos, (float)ypos);
}

void rift_glfw_scroll_event_callback(GLFWwindow* window, double xoffset, double yoffset) {
    OS_InputScrollCallback((float)xoffset, (float)yoffset);
}

void rift_glfw_key_event_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Rift_Window* data = glfwGetWindowUserPointer(window);
    
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_F11) {
            if (!data->props.is_fullscreen) {
                const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
                glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
            } else {
                glfwSetWindowMonitor(window, nullptr, 100, 100, 1080, 720, GLFW_DONT_CARE);
            }
            data->props.is_fullscreen = !data->props.is_fullscreen;
        }
    }
    
    OS_InputKeyCallback(key, action);
}


//- Actual Stuff

void Rift_WindowCreate(Rift_Window* win, Rift_WindowProps props) {
    glfwInit();
    
    win->props = props;
    
    if (props.custom_titlebar) {
        glfwWindowHint(GLFW_TITLEBAR, GLFW_FALSE);
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    win->ref = glfwCreateWindow(props.width, props.height, (char*)props.name.str,
                                nullptr, nullptr);
    
    if (props.custom_titlebar) {
        glfwSetTitlebarHitTestCallback(win->ref, rift_glfw_titlebar_hittest);
    }
    glfwSetWindowSizeCallback(win->ref, rift_glfw_window_size_callback);
    glfwSetMouseButtonCallback(win->ref, rift_glfw_mouse_event_callback);
    glfwSetKeyCallback(win->ref, rift_glfw_key_event_callback);
    glfwSetCursorPosCallback(win->ref, rift_glfw_cursor_pos_event_callback);
    glfwSetScrollCallback(win->ref, rift_glfw_scroll_event_callback);
    
    OS_InputSetupWindowRef(win->ref);
    OS_InputProject(1080, 720, 1080, 720);
    
    glfwMakeContextCurrent(win->ref);
    gladLoadGL();
    
    glClearColor(0.1, 0.1, 0.14, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glfwSetWindowUserPointer(win->ref, win);
}



static Rift_UIBoxStyle g_titlebar_main_style = (Rift_UIBoxStyle) {
    .color        = (vec4){0.10f, 0.10f, 0.12f, 1.0f},
    .hot_color    = (vec4){0.15f, 0.15f, 0.17f, 1.0f},
    .active_color = (vec4){0.06f, 0.06f, 0.12f, 1.0f},
    
    .border_color        = (vec4){0.20f, 0.40f, 0.52f, 1.0f},
    .border_hot_color    = (vec4){0.20f, 0.40f, 0.52f, 1.0f},
    .border_active_color = (vec4){0.06f, 0.06f, 0.15f, 1.0f},
    
    .text_color = (vec4){0.8f, 0.8f, 0.94f, 1.0f},
    .font = 0,
    .font_size = 20,
    
    .rounding = 0,
    .softness = 0.1,
    .edge_size = 0,
};


Rift_UIBox* Rift_WindowCustomTitlebar(Rift_Window* win, Rift_UIContext* ctx, Rift_UISimpleRenderer* renderer,
                                      Rift_TriRenderer* trirenderer) {
    win->ui = ctx;
    win->ui_renderer = renderer;
    win->tri_renderer = trirenderer;
    
    ctx->container->layout_axis = UIAxis_Y;
    
    win->top_container = Rift_UIBoxCreate(ctx, ctx->container,
                                          Rift_UISizePct(1), Rift_UISizePixels(30),
                                          &g_titlebar_main_style,
                                          UIBoxFlag_DrawBack | UIBoxFlag_Interactive |
                                          UIBoxFlag_Instant  | UIBoxFlag_NoHotAnim);
    win->top_container->layout_axis = UIAxis_X;
    win->top_container->alignment   = UIAlign_End;
    
    win->minimize_button = Rift_UIBoxCreate(ctx, win->top_container,
                                            Rift_UISizePixels(30), Rift_UISizePixels(30),
                                            &g_titlebar_main_style,
                                            UIBoxFlag_DrawBack | UIBoxFlag_DrawBorder  |
                                            UIBoxFlag_DrawName | UIBoxFlag_Interactive |
                                            UIBoxFlag_Instant);
    win->minimize_button->text = str_lit("_");
    
    win->close_button = Rift_UIBoxCreate(ctx, win->top_container,
                                         Rift_UISizePixels(30), Rift_UISizePixels(30),
                                         &g_titlebar_main_style,
                                         UIBoxFlag_DrawBack | UIBoxFlag_DrawBorder  |
                                         UIBoxFlag_DrawName | UIBoxFlag_Interactive |
                                         UIBoxFlag_Instant);
    win->close_button->text = str_lit("x");
    win->close_button->style.hot_color = (vec4){0.3f, 0.2f, 0.2f, 1.0f};
    
    Rift_UIBox* main_container = Rift_UIBoxCreate(ctx, ctx->container,
                                                  Rift_UISizePct(1), Rift_UISizePixels(-30),
                                                  &g_titlebar_main_style,
                                                  UIBoxFlag_Instant);
    return main_container;
}

void Rift_WindowUpdateTitlebar(Rift_Window* win, Rift_UIContext* ctx) {
    if (Rift_UIBoxSignal(ctx, win->close_button).clicked) {
        Rift_WindowTriggerExit(win);
    }
    
    if (Rift_UIBoxSignal(ctx, win->minimize_button).clicked) {
        Rift_WindowMinimize(win);
    }
}

void Rift_WindowDestroy(Rift_Window* win) {
    glfwDestroyWindow(win->ref);
    
    glfwTerminate();
}

void Rift_WindowMinimize(Rift_Window* win) {
    glfwIconifyWindow(win->ref);
}

void Rift_WindowRestore(Rift_Window* win) {
    glfwRestoreWindow(win->ref);
}

b8 Rift_WindowIsOpen(Rift_Window* win) {
    static b8 wack_fix = false;
    if (!wack_fix) {
        glfwSetWindowSize(win->ref, win->props.width, win->props.height+1);
        glfwSetWindowSize(win->ref, win->props.width, win->props.height);
        wack_fix = true;
    }
    
    return !glfwWindowShouldClose(win->ref);
}

void Rift_WindowTriggerExit(Rift_Window* win) {
    glfwSetWindowShouldClose(win->ref, true);
}

void Rift_PollEvents() {
    glfwPollEvents();
}

void Rift_WindowSwapBuffers(Rift_Window* win) {
    glfwSwapBuffers(win->ref);
}