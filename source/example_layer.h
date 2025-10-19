/* date = September 27th 2025 8:46 pm */

#ifndef CSM_LAYER_H
#define CSM_LAYER_H

#include "defines.h"

#include "client/ui.h"
#include "client/simple_ui_render.h"

void CSM_Init(Rift_UIContext* ui, Rift_UIBox* content);

void CSM_Update(f32 delta);
void CSM_Render();

void CSM_Free();

#endif //CSM_LAYER_H
