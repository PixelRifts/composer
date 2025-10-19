#version 400 core

in vec2 v_uv;

layout (location=0) out vec4 frag_color;

uniform sampler2D u_texture;

void main() {
  frag_color = vec4(texture(u_texture, v_uv).rgb, 1);
}