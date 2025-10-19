/* date = September 22nd 2025 2:27 pm */

#ifndef INPUT_H
#define INPUT_H

#include "defines.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "base/vmath.h"

void OS_InputProject(f32 window_width, f32 window_height, f32 width, f32 height);
void OS_InputKeyCallback(u16 key, i32 action);
b8   OS_InputKeyCallbackCheckRepeat(u16 key, i32 action);
void OS_InputButtonCallback(u16 button, i32 action);
void OS_InputCursorPosCallback(f32 xpos, f32 ypos);
void OS_InputScrollCallback(f32 xscroll, f32 yscroll);
void OS_InputReset(void);

b8  OS_InputKey(u16 key);
b8  OS_InputKeyPressed(u16 key);
b8  OS_InputKeyReleased(u16 key);
b8  OS_InputKeyHeld(u16 key);
b8  OS_InputMouseButton(u16 button);
b8  OS_InputMouseButtonPressed(u16 button);
b8  OS_InputMouseButtonReleased(u16 button);
f32 OS_InputMouseX();
f32 OS_InputMouseY();
f32 OS_InputMouseScrollX();
f32 OS_InputMouseScrollY();
f32 OS_InputMouseAbsoluteScrollX();
f32 OS_InputMouseAbsoluteScrollY();
f32 OS_InputMouseDX();
f32 OS_InputMouseDY();
f32 OS_InputMouseRecordedX();
f32 OS_InputMouseRecordedY();

void OS_InputSetupWindowRef(GLFWwindow* window);
vec2 OS_InputDirectMousePos();


#endif //INPUT_H
