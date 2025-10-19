#include "tri_render.h"
#include <glad/glad.h>

#include <stdint.h>
#include <string.h>
#include <stb/stb_image.h>

static const char* vert_shader_source = "#version 330 core\n"
"\n"
"layout (location=0) in vec2  a_pos;\n"
"layout (location=1) in vec2  a_uv;\n"
"layout (location=2) in vec4  a_color;\n"
"layout (location=3) in float a_tex_idx;\n"
"\n"
"out vec2  v_uv;\n"
"out vec4  v_color;\n"
"out float v_tex_idx;"
"\n"
"uniform mat4 u_proj;\n"
"\n"
"void main() {\n"
"  gl_Position = u_proj * vec4(a_pos, 0., 1.);\n"
"  \n"
"  v_uv = a_uv;\n"
"  v_color = a_color;\n"
"  v_tex_idx = a_tex_idx;\n"
"}\n"
"\n";

static const char* frag_shader_source = "#version 330 core\n"
"\n"
"in vec2  v_uv;\n"
"in vec4  v_color;\n"
"in float v_tex_idx;\n"
"\n"
"layout (location=0) out vec4 f_color;\n"
"\n"
"uniform sampler2D u_textures[16];\n"
"\n"
"void main() {\n"
"  vec4 sample = texture(u_textures[int(v_tex_idx)], v_uv);\n"
"  f_color = v_color * sample;\n"
"}\n"
"\n";


Rift_TriRenderer* Rift_TriRendererInit(M_Arena* arena, f32 window_width, f32 window_height) {
    Rift_TriRenderer* ret = arena_alloc_zero(arena, sizeof(Rift_TriRenderer));
    glGenVertexArrays(1, &ret->vao);
    glBindVertexArray(ret->vao);
    
    glGenBuffers(1, &ret->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ret->vbo);
    glBufferData(GL_ARRAY_BUFFER, RTRI_MAX_VERTICES * sizeof(Rift_TriVertex),
                 nullptr, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Rift_TriVertex), (void*) offsetof(Rift_TriVertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Rift_TriVertex), (void*) offsetof(Rift_TriVertex, uv));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Rift_TriVertex), (void*) offsetof(Rift_TriVertex, color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Rift_TriVertex), (void*) offsetof(Rift_TriVertex, tex_idx));
    glEnableVertexAttribArray(3);
    
    uint8_t white_data[4] = { 255, 255, 255, 255 };
    ret->texture_count = 1;
    glGenTextures(1, &ret->textures[0]);
    glBindTexture(GL_TEXTURE_2D, ret->textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, white_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    ret->shader = glCreateProgram();
    uint32_t vert_module = glCreateShader(GL_VERTEX_SHADER);
    uint32_t frag_module = glCreateShader(GL_FRAGMENT_SHADER);
    int vert_shader_source_size = strlen(vert_shader_source);
    int frag_shader_source_size = strlen(frag_shader_source);
    glShaderSource(vert_module, 1, (const GLchar* const*) &vert_shader_source, &vert_shader_source_size);
    glShaderSource(frag_module, 1, (const GLchar* const*) &frag_shader_source, &frag_shader_source_size);
    glCompileShader(vert_module);
    glCompileShader(frag_module);
    
    int error = 0;
    glGetShaderiv(vert_module, GL_COMPILE_STATUS, &error);
    if (error == GL_FALSE) {
        printf("Vertex Shader Compilation failed!\n");
        int length = 0;
        glGetShaderiv(vert_module, GL_INFO_LOG_LENGTH, &length);
        
        GLchar* info = arena_alloc(arena, length * sizeof(GLchar));
        glGetShaderInfoLog(vert_module, length * sizeof(GLchar), nullptr, info);
        printf("%s", info);
    }
    
    glGetShaderiv(frag_module, GL_COMPILE_STATUS, &error);
    if (error == GL_FALSE) {
        printf("Fragment Shader Compilation failed!\n");
        int length = 0;
        glGetShaderiv(frag_module, GL_INFO_LOG_LENGTH, &length);
        
        GLchar* info = arena_alloc(arena, length * sizeof(GLchar));
        glGetShaderInfoLog(frag_module, length * sizeof(GLchar), nullptr, info);
        printf("%s", info);
    }
    
    glAttachShader(ret->shader, vert_module);
    glAttachShader(ret->shader, frag_module);
    glLinkProgram(ret->shader);
    
    glGetProgramiv(ret->shader, GL_COMPILE_STATUS, &error);
    if (error == GL_FALSE) {
        printf("Program Link failed!\n");
        int length = 0;
        glGetProgramiv(ret->shader, GL_INFO_LOG_LENGTH, &length);
        
        GLchar* info = arena_alloc(arena, length * sizeof(GLchar));
        glGetProgramInfoLog(ret->shader, length * sizeof(GLchar), nullptr, info);
        printf("%s", info);
    }
    
    glDetachShader(ret->shader, vert_module);
    glDetachShader(ret->shader, frag_module);
    glDeleteShader(vert_module);
    glDeleteShader(frag_module);
    
    
    float width = (window_width - 0);
    float height = (0 - window_height);
    float depth = (1000.0f + 1.0f);
    ret->matrix[0] = 2.f / width;
    ret->matrix[1] = 0.f;
    ret->matrix[2] = 0.f;
    ret->matrix[3] = 0.f;
    ret->matrix[4] = 0.f;
    ret->matrix[5] = 2.f / height;
    ret->matrix[6] = 0.f;
    ret->matrix[7] = 0.f;
    ret->matrix[8 ] = 0.f;
    ret->matrix[9 ] = 0.f;
    ret->matrix[10] = -2.f / depth;
    ret->matrix[11] = 0.f;
    ret->matrix[12] = -(window_width + 0)  / width;
    ret->matrix[13] = -(0 + window_height) / height;
    ret->matrix[14] = -(1000.f - 1.f)      / depth;
    ret->matrix[15] = 1.f;
    
    glUseProgram(ret->shader);
    glUniformMatrix4fv(glGetUniformLocation(ret->shader, "u_proj"), 1, GL_FALSE, ret->matrix);
    int32_t textures[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    glUniform1iv(glGetUniformLocation(ret->shader, "u_textures"), 8, textures);
    
    arena_init(&ret->arena);
    
    return ret;
}

void Rift_TriRendererResize(Rift_TriRenderer* renderer, float window_width, float window_height) {
    float width = (window_width - 0);
    float height = (0 - window_height);
    float depth = (1000.0f + 1.0f);
    renderer->matrix[0 ] = 2.f / width;
    renderer->matrix[1 ] = 0.f;
    renderer->matrix[2 ] = 0.f;
    renderer->matrix[3 ] = 0.f;
    renderer->matrix[4 ] = 0.f;
    renderer->matrix[5 ] = 2.f / height;
    renderer->matrix[6 ] = 0.f;
    renderer->matrix[7 ] = 0.f;
    renderer->matrix[8 ] = 0.f;
    renderer->matrix[9 ] = 0.f;
    renderer->matrix[10] = -2.f / depth;
    renderer->matrix[11] = 0.f;
    renderer->matrix[12] = -(window_width + 0)  / width;
    renderer->matrix[13] = -(0 + window_height) / height;
    renderer->matrix[14] = -(1000.f - 1.f)      / depth;
    renderer->matrix[15] = 1.f;
    
    glUseProgram(renderer->shader);
    glUniformMatrix4fv(glGetUniformLocation(renderer->shader, "u_proj"), 1, GL_FALSE, renderer->matrix);
}

void Rift_TriRendererBegin(Rift_TriRenderer* renderer) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    renderer->triangle_count = 0;
    renderer->texture_count = 1;
}

void Rift_TriRendererEnd(Rift_TriRenderer* renderer) {
    for (int i = 0; i < renderer->texture_count; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[i]);
    }
    glUseProgram(renderer->shader);
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->triangle_count*3*sizeof(Rift_TriVertex), renderer->vertices);
    glDrawArrays(GL_TRIANGLES, 0, renderer->triangle_count * 3);
}

void Rift_TriRendererDrawTri(Rift_TriRenderer* renderer,
                             Rift_TriVertex p1, Rift_TriVertex p2,
                             Rift_TriVertex p3, uint32_t texture) {
    
    int tex_id = -1;
    for (int i = 0; i < renderer->texture_count; i++) {
        if (renderer->textures[i] == texture) {
            tex_id = i;
            break;
        }
    }
    if (tex_id == -1 && renderer->texture_count < 16) {
        renderer->textures[renderer->texture_count] = texture;
        tex_id = renderer->texture_count;
        renderer->texture_count += 1;
    }
    
    if (renderer->triangle_count > RTRI_MAX_TRIANGLES ||
        renderer->texture_count  >= 16) {
        Rift_TriRendererEnd(renderer);
        Rift_TriRendererBegin(renderer);
        
        for (int i = 0; i < renderer->texture_count; i++) {
            if (renderer->textures[i] == texture) {
                tex_id = i;
                break;
            }
        }
        if (tex_id == -1) {
            renderer->textures[renderer->texture_count] = texture;
            tex_id = renderer->texture_count;
            renderer->texture_count += 1;
        }
    }
    
    p1.tex_idx = (float)tex_id;
    p2.tex_idx = (float)tex_id;
    p3.tex_idx = (float)tex_id;
    
    renderer->vertices[renderer->triangle_count * 3 + 0] = p1;
    renderer->vertices[renderer->triangle_count * 3 + 1] = p2;
    renderer->vertices[renderer->triangle_count * 3 + 2] = p3;
    renderer->triangle_count += 1;
}


void Rift_TriRendererDrawLine(Rift_TriRenderer* renderer,
                              vec2 start, vec2 end, vec4 color, f32 thickness) {
    vec2 half_thick = vec2_scale(vec2_normalize(vec2_sub(end, start)), thickness / 2.0f);
    half_thick = v2(half_thick.y, -half_thick.x);
    Rift_TriRendererDrawTri(renderer,
                            (Rift_TriVertex) {
                                .pos = vec2_add(start, half_thick),
                                .uv = v2(0.f, 0.f),
                                .color = color,
                                .tex_idx = -1,
                            },
                            (Rift_TriVertex) {
                                .pos = vec2_add(end, half_thick),
                                .uv = v2(1.f, 0.f),
                                .color = color,
                                .tex_idx = -1,
                            },
                            (Rift_TriVertex) {
                                .pos = vec2_sub(end, half_thick),
                                .uv = v2(1.f, 1.f),
                                .color = color,
                                .tex_idx = -1,
                            }, renderer->textures[0]);
    Rift_TriRendererDrawTri(renderer,
                            (Rift_TriVertex) {
                                .pos = vec2_add(start, half_thick),
                                .uv = v2(0.f, 0.f),
                                .color = color,
                                .tex_idx = -1,
                            },
                            (Rift_TriVertex) {
                                .pos = vec2_sub(end, half_thick),
                                .uv = v2(1.f, 1.f),
                                .color = color,
                                .tex_idx = -1,
                            },
                            (Rift_TriVertex) {
                                .pos = vec2_sub(start, half_thick),
                                .uv = v2(0.f, 1.f),
                                .color = color,
                                .tex_idx = -1,
                            }, renderer->textures[0]);
}

void Rift_TriRendererFree(Rift_TriRenderer* renderer) {
    glDeleteProgram(renderer->shader);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteVertexArrays(1, &renderer->vao);
    
    arena_free(&renderer->arena);
}