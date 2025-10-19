#include "input.h"
#include <string.h>

typedef struct input_state {
    u8 key_frame_states[350];
    u8 key_current_states[350];
    u8 button_frame_states[8];
    u8 button_current_states[8];
    
    f32 mouse_x;
    f32 mouse_y;
    f32 mouse_scrollx;
    f32 mouse_scrolly;
    f32 mouse_absscrollx;
    f32 mouse_absscrolly;
    f32 mouse_recordedx;
    f32 mouse_recordedy;
    
    f32 map_width;
    f32 map_height;
    
    GLFWwindow* window;
} input_state;
static input_state _state;


void OS_InputProject(f32 window_width, f32 window_height, f32 width, f32 height) {
    _state.map_width = width / window_width;
    _state.map_height = height / window_height;
}

void OS_InputKeyCallback(u16 key, i32 action) {
    switch (action) {
        case GLFW_PRESS: {
			_state.key_frame_states[key] = 0b00000001;
			_state.key_current_states[key] = 1;
		} break;
		
		case GLFW_RELEASE: {
			_state.key_frame_states[key] = 0b00000010;
			_state.key_current_states[key] = 0;
		} break;
		
		case GLFW_REPEAT: {
			_state.key_frame_states[key] = 0b00000100;
		} break;
	}
}

b8 OS_InputKeyCallbackCheckRepeat(u16 key, i32 action) {
    b8 did_repeat = false;
	if (action == GLFW_PRESS) {
		if (_state.key_current_states[key] == 1) {
			action = GLFW_REPEAT;
			did_repeat = true;
		}
	}
	
    switch (action) {
        case GLFW_PRESS: {
			_state.key_frame_states[key] = 0b00000001;
			_state.key_current_states[key] = 1;
		} break;
		
		case GLFW_RELEASE: {
			_state.key_frame_states[key] = 0b00000010;
			_state.key_current_states[key] = 0;
		} break;
		
		case GLFW_REPEAT: {
			_state.key_frame_states[key] = 0b00000100;
		} break;
	}
	
	return did_repeat;
}

void OS_InputButtonCallback(u16 button, int action) {
    if (button < 0 || button >= 8) return;
    switch (action) {
        case GLFW_PRESS: {
            _state.button_frame_states[button] = 0b00000001;
            _state.button_current_states[button] = 1;
            _state.mouse_recordedx = _state.mouse_x;
            _state.mouse_recordedy = _state.mouse_y;
        } break;
        case GLFW_RELEASE: {
            _state.button_frame_states[button] = 0b00000010;
			_state.button_current_states[button] = 0;
            _state.mouse_recordedx = _state.mouse_x;
            _state.mouse_recordedy = _state.mouse_y;
        } break;
    }
}

void OS_InputCursorPosCallback(f32 xpos, f32 ypos) {
    _state.mouse_x = xpos * _state.map_width;
    _state.mouse_y = ypos * _state.map_height;
}

void OS_InputScrollCallback(f32 xscroll, f32 yscroll) {
    _state.mouse_scrollx = xscroll;
    _state.mouse_scrolly = yscroll;
    _state.mouse_absscrollx += xscroll;
    _state.mouse_absscrolly += yscroll;
}

void OS_InputReset(void) {
    memset(_state.key_frame_states, 0, 350 * sizeof(u8));
    memset(_state.button_frame_states, 0, 8 * sizeof(u8));
    _state.mouse_scrollx = 0;
    _state.mouse_scrolly = 0;
}

b8 OS_InputKey(u16 key) { return _state.key_current_states[key]; }
b8 OS_InputKeyPressed(u16 key) { return (_state.key_frame_states[key] == 0b00000001); }
b8 OS_InputKeyReleased(u16 key) { return (_state.key_frame_states[key] == 0b00000010); }
b8 OS_InputKeyHeld(u16 key) { return (_state.key_frame_states[key] == 0b00000100); }
b8 OS_InputMouseButton(u16 button) { return _state.button_current_states[button]; }

b8 OS_InputMouseButtonPressed(u16 button) {
    return (_state.button_frame_states[button] == 0b00000001);
}
b8 OS_InputMouseButtonReleased(u16 button) {
    return (_state.button_frame_states[button] == 0b00000010);
}

void OS_InputSetupWindowRef(GLFWwindow* window) {
    _state.window = window;
}

vec2 OS_InputDirectMousePos() {
    double xp, yp;
    glfwGetCursorPos(_state.window, &xp, &yp);
    return v2(xp, yp);
}
f32 OS_InputMouseX() { return _state.mouse_x; }
f32 OS_InputMouseY() { return _state.mouse_y; }
f32 OS_InputMouseScrollX() { return _state.mouse_scrollx; }
f32 OS_InputMouseScrollY() { return _state.mouse_scrolly; }
f32 OS_InputMouseAbsoluteScrollX() { return _state.mouse_absscrollx; }
f32 OS_InputMouseAbsoluteScrollY() { return _state.mouse_absscrolly; }

f32 OS_InputMouseDX() {
    if (!OS_InputMouseButton(GLFW_MOUSE_BUTTON_LEFT)) return 0.0f;
    return _state.mouse_x - _state.mouse_recordedx;
}
f32 OS_InputMouseDY() {
    if (!OS_InputMouseButton(GLFW_MOUSE_BUTTON_LEFT)) return 0.0f;
    return _state.mouse_y - _state.mouse_recordedy;
}

f32 OS_InputMouseRecordedX() { return _state.mouse_recordedx; }
f32 OS_InputMouseRecordedY() { return _state.mouse_recordedy; }
