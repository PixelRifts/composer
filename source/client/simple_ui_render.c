#include "simple_ui_render.h"
#include <glad/glad.h>

#include <stdint.h>
#include <string.h>
#include <stb/stb_image.h>


static const char* vert_shader_source = "#version 330 core\n"
"\n"
"layout (location=0) in vec4 a_pos_size;\n"
"layout (location=1) in vec2 a_uv;\n"
"layout (location=2) in vec4 a_color;\n"
"layout (location=3) in vec3 a_rounding_params;\n"
"layout (location=4) in vec2 a_tex_params;\n"
"\n"
"out vec4 v_center_halfsize;\n"
"out vec2 v_uv;\n"
"out vec4 v_color;\n"
"out vec3 v_rounding_params;\n"
"out vec2 v_sampling_loc;\n"
"out vec2 v_tex_params;"
"\n"
"const vec2 vertex_mults[6] = vec2[6](\n"
"  vec2(0.0, 0.0),\n"
"  vec2(1.0, 0.0),\n"
"  vec2(1.0, 1.0),\n"
"  vec2(0.0, 0.0),\n"
"  vec2(1.0, 1.0),\n"
"  vec2(0.0, 1.0)\n"
");\n"
"\n"
"uniform mat4 u_proj;\n"
"\n"
"void main() {\n"
"  vec2 offset = vertex_mults[gl_VertexID % 6] * a_pos_size.zw;\n"
"  vec2 pos = a_pos_size.xy + offset;\n"
"  gl_Position = u_proj * vec4(pos, 0., 1.);\n"
"  \n"
"  vec2 half_size = 0.5 * a_pos_size.zw;"
"  v_center_halfsize = vec4(a_pos_size.xy+half_size, half_size);\n"
"  v_uv = a_uv;\n"
"  v_color = a_color;\n"
"  v_rounding_params = a_rounding_params;\n"
"  v_sampling_loc = pos;\n"
"  v_tex_params = a_tex_params;\n"
"}\n"
"\n";


static const char* frag_shader_source = "#version 330 core\n"
"\n"
"in vec4 v_center_halfsize;\n"
"in vec2 v_uv;\n"
"in vec4 v_color;\n"
"in vec3 v_rounding_params;\n"
"in vec2 v_sampling_loc;\n"
"in vec2 v_tex_params;\n"
"\n"
"layout (location=0) out vec4 f_color;\n"
"\n"
"uniform sampler2D u_textures[16];\n"
"\n"
"float rounded_rect(vec2 sample_pos, vec2 rect_center, vec2 rect_half_size,\n"
"                     float r) {\n"
"  vec2 d2 = (abs(rect_center - sample_pos) - rect_half_size + vec2(r, r));\n"
"  return min(max(d2.x, d2.y), 0.0) + length(max(d2, 0.0)) - r;\n"
"}\n"
"\n"
"\n"
"\n"
"float screenPxRange() {\n"
"  const float pxRange = 2.0; // set to distance field's pixel range\n"
"  vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_textures[int(v_tex_params.x)], 0));\n"
"  vec2 screenTexSize = vec2(1.0)/fwidth(v_uv);\n"
"  return max(0.5*dot(unitRange, screenTexSize), 1.0);\n"
"}\n"
"\n"
"\n"
"float median(float r, float g, float b) {\n"
"  return max(min(r, g), min(max(r, g), b));\n"
"}\n"
"\n"
"void main() {\n"
"  f_color = v_color;\n"
"  float softness = v_rounding_params.y;\n"
"  vec2  softness_padding = vec2(max(0, softness*2.-1.), max(0, softness*2.-1.));\n"
"  \n"
"  float dist = rounded_rect(v_sampling_loc, v_center_halfsize.xy,\n"
"                            v_center_halfsize.zw - softness_padding,\n"
"                            v_rounding_params.x);\n"
"  float sdf_factor = 1.0 - smoothstep(0, 2*softness, dist);\n"
"  f_color *= sdf_factor;\n"
"  \n"
"  vec4 sample = texture(u_textures[int(v_tex_params.x)], v_uv);\n"
"  \n"
"  if (v_tex_params.y == 1.0) {"
"    vec3 msd = sample.rgb;\n"
"    float sd = median(msd.r, msd.g, msd.b);\n"
"    float screenPxDistance = screenPxRange()*(sd - 0.5);\n"
"    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);\n"
"    f_color.a *= opacity;\n"
"  } else {\n"
"    f_color *= sample;\n"
"  }\n"
"}\n"
"\n";


Rift_UISimpleRenderer* Rift_UIRendererInit(M_Arena* arena, f32 window_width, f32 window_height) {
    Rift_UISimpleRenderer* ret = arena_alloc_zero(arena, sizeof(Rift_UISimpleRenderer));
    glGenVertexArrays(1, &ret->vao);
    glBindVertexArray(ret->vao);
    
    glGenBuffers(1, &ret->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ret->vbo);
    glBufferData(GL_ARRAY_BUFFER, QUI_MAX_VERTICES * sizeof(Rift_UISimpleVertex),
                 nullptr, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Rift_UISimpleVertex), (void*) offsetof(Rift_UISimpleVertex, pos_size));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Rift_UISimpleVertex), (void*) offsetof(Rift_UISimpleVertex, uv));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Rift_UISimpleVertex), (void*) offsetof(Rift_UISimpleVertex, color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Rift_UISimpleVertex), (void*) offsetof(Rift_UISimpleVertex, rounding_params));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Rift_UISimpleVertex), (void*) offsetof(Rift_UISimpleVertex, tex_params));
    glEnableVertexAttribArray(4);
    
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
    ret->font_count = 0;
    
    return ret;
}

void Rift_UIRendererResize(Rift_UISimpleRenderer* renderer, float window_width, float window_height) {
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

void Rift_UIRendererBegin(Rift_UISimpleRenderer* renderer) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    renderer->triangle_count = 0;
    renderer->texture_count = 1;
}

void Rift_UIRendererEnd(Rift_UISimpleRenderer* renderer) {
    for (int i = 0; i < renderer->texture_count; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[i]);
    }
    glUseProgram(renderer->shader);
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->triangle_count*3*sizeof(Rift_UISimpleVertex), renderer->vertices);
    glDrawArrays(GL_TRIANGLES, 0, renderer->triangle_count * 3);
}

void Rift_UIRendererDrawQuad(Rift_UISimpleRenderer* renderer,
                             Rift_UISimpleVertex p1, Rift_UISimpleVertex p2,
                             Rift_UISimpleVertex p3, Rift_UISimpleVertex p4,
                             uint32_t texture) {
    
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
    
    if (renderer->triangle_count > QUI_MAX_TRIANGLES ||
        renderer->texture_count  > 16) {
        Rift_UIRendererEnd(renderer);
        Rift_UIRendererBegin(renderer);
        
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
    
    p1.tex_params.x = (float)tex_id;
    p2.tex_params.x = (float)tex_id;
    p3.tex_params.x = (float)tex_id;
    p4.tex_params.x = (float)tex_id;
    
    renderer->vertices[renderer->triangle_count * 3 + 0] = p1;
    renderer->vertices[renderer->triangle_count * 3 + 1] = p2;
    renderer->vertices[renderer->triangle_count * 3 + 2] = p3;
    renderer->triangle_count += 1;
    renderer->vertices[renderer->triangle_count * 3 + 0] = p1;
    renderer->vertices[renderer->triangle_count * 3 + 1] = p3;
    renderer->vertices[renderer->triangle_count * 3 + 2] = p4;
    renderer->triangle_count += 1;
}


void Rift_UIRendererDrawBox(Rift_UISimpleRenderer* renderer, Rift_UIBox* box) {
    if (box->flags & UIBoxFlag_DrawBorder) {
        float x = box->render_pos.x;
        float y = box->render_pos.y;
        float width  = box->render_width;
        float height = box->render_height;
        
        Rift_UIRendererDrawQuad(renderer, (Rift_UISimpleVertex) {
                                    v4(x, y, width, height),
                                    v2(0.0f, 0.0f),
                                    box->render_style.border_color,
                                    v3(box->render_style.rounding+box->render_style.edge_size, box->render_style.softness,
                                       box->render_style.edge_size),
                                    v2(0.0f, 0.0f),
                                }, (Rift_UISimpleVertex) {
                                    v4(x, y, width, height),
                                    v2(1.0f, 0.0f),
                                    box->render_style.border_color,
                                    v3(box->render_style.rounding+box->render_style.edge_size, box->render_style.softness,
                                       box->render_style.edge_size),
                                    v2(0.0f, 0.0f),
                                }, (Rift_UISimpleVertex) {
                                    v4(x, y, width, height),
                                    v2(1.0f, 1.0f),
                                    box->render_style.border_color,
                                    v3(box->render_style.rounding+box->render_style.edge_size, box->render_style.softness,
                                       box->render_style.edge_size),
                                    v2(0.0f, 0.0f),
                                }, (Rift_UISimpleVertex) {
                                    v4(x, y, width, height),
                                    v2(0.0f, 1.0f),
                                    box->render_style.border_color,
                                    v3(box->render_style.rounding+box->render_style.edge_size, box->render_style.softness,
                                       box->render_style.edge_size),
                                    v2(0.0f, 0.0f),
                                }, renderer->textures[0]);
    }
    
    if (box->flags & UIBoxFlag_DrawBack) {
        float x = box->render_pos.x + box->render_style.edge_size;
        float y = box->render_pos.y + box->render_style.edge_size;
        float width  = box->render_width  - box->render_style.edge_size * 2.0f;
        float height = box->render_height - box->render_style.edge_size * 2.0f;
        
        Rift_UIRendererDrawQuad(renderer, (Rift_UISimpleVertex) {
                                    v4(x, y, width, height),
                                    v2(0.0f, 0.0f),
                                    box->render_style.color,
                                    v3(box->render_style.rounding+box->render_style.edge_size, box->render_style.softness,
                                       box->render_style.edge_size),
                                    v2(0.0f, 0.0f),
                                }, (Rift_UISimpleVertex) {
                                    v4(x, y, width, height),
                                    v2(1.0f, 0.0f),
                                    box->render_style.color,
                                    v3(box->render_style.rounding, box->render_style.softness,
                                       box->render_style.edge_size),
                                    v2(0.0f, 0.0f),
                                }, (Rift_UISimpleVertex) {
                                    v4(x, y, width, height),
                                    v2(1.0f, 1.0f),
                                    box->render_style.color,
                                    v3(box->render_style.rounding, box->render_style.softness,
                                       box->render_style.edge_size),
                                    v2(0.0f, 0.0f),
                                }, (Rift_UISimpleVertex) {
                                    v4(x, y, width, height),
                                    v2(0.0f, 1.0f),
                                    box->render_style.color,
                                    v3(box->render_style.rounding, box->render_style.softness,
                                       box->render_style.edge_size),
                                    v2(0.0f, 0.0f),
                                }, renderer->textures[0]);
    }
    
    
    if (box->flags & UIBoxFlag_DrawName) {
        Rift_UIFont* font = &renderer->fonts[box->render_style.font];
        
        float max_height = 0.0f;
        float text_width = 0.0f;
        for (int i = 0; i < box->text.size; i++) {
            Rift_UIFontGlyph* glyph = &font->glyphs[(box->text.str[i])-32];
            max_height = Max(max_height, ((glyph->top-glyph->bottom) * box->render_style.font_size));
            text_width += (glyph->right-glyph->left) * box->render_style.font_size;
        }
        
        float xo = (box->render_width - text_width) / 2.0f;
        for (int i = 0; i < box->text.size; i++) {
            Rift_UIFontGlyph* glyph = &font->glyphs[(box->text.str[i])-32];
            
            float width =  (glyph->right-glyph->left) * box->render_style.font_size;
            float height = (glyph->top-glyph->bottom) * box->render_style.font_size;
            
            float x = box->render_pos.x + glyph->left + xo;
            float y = box->render_pos.y + max_height - height - glyph->bottom * box->render_style.font_size;
            y += box->render_height / 2.0f - max_height / 2.0f;
            
            float uv_left = glyph->atlas_left / font->texture_width;
            float uv_bot = glyph->atlas_bottom / font->texture_height;
            float uv_right = glyph->atlas_right / font->texture_width;
            float uv_top = glyph->atlas_top / font->texture_height;
            
            Rift_UIRendererDrawQuad(renderer, (Rift_UISimpleVertex) {
                                        v4(x, y, width, height),
                                        v2(uv_left, uv_top),
                                        v4(box->render_style.text_color.x, box->render_style.text_color.y,
                                           box->render_style.text_color.z, box->render_style.text_color.w),
                                        v3(0.0f, 0.0f, 0.0f),
                                        v2(0.0f, 1.0f),
                                    }, (Rift_UISimpleVertex) {
                                        v4(x, y, width, height),
                                        v2(uv_right, uv_top),
                                        v4(box->render_style.text_color.x, box->render_style.text_color.y,
                                           box->render_style.text_color.z, box->render_style.text_color.w),
                                        v3(0.0f, 0.0f, 0.0f),
                                        v2(0.0f, 1.0f),
                                    }, (Rift_UISimpleVertex) {
                                        v4(x, y, width, height),
                                        v2(uv_right, uv_bot),
                                        v4(box->render_style.text_color.x, box->render_style.text_color.y,
                                           box->render_style.text_color.z, box->render_style.text_color.w),
                                        v3(0.0f, 0.0f, 0.0f),
                                        v2(0.0f, 1.0f),
                                    }, (Rift_UISimpleVertex) {
                                        v4(x, y, width, height),
                                        v2(uv_left, uv_bot),
                                        v4(box->render_style.text_color.x, box->render_style.text_color.y,
                                           box->render_style.text_color.z, box->render_style.text_color.w),
                                        v3(0.0f, 0.0f, 0.0f),
                                        v2(0.0f, 1.0f),
                                    }, font->texture);
            
            xo += glyph->advance * box->render_style.font_size;
        }
    }
}


void Rift_UIRendererFree(Rift_UISimpleRenderer* renderer) {
    glDeleteProgram(renderer->shader);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteVertexArrays(1, &renderer->vao);
    
    arena_free(&renderer->arena);
}

u32 Rift_UIFontLoad(Rift_UISimpleRenderer* renderer, char* name) {
    if (renderer->font_count == 8) {
        printf("Too many fonts\n");
        exit(1);
    }
    
    u32 id = renderer->font_count;
    
    // Texture Loading
    char* png_name = arena_alloc(&renderer->arena, sizeof("assets/")-1 + strlen(name) + sizeof(".png")-1 + 1);
    int i = 0;
    memcpy(&png_name[i], "assets/", sizeof("assets/")-1); i += sizeof("assets/")-1;
    memcpy(&png_name[i], name, strlen(name)); i += strlen(name);
    memcpy(&png_name[i], ".png", sizeof(".png")-1); i += sizeof(".png")-1;
    png_name[i] = '\0';
    
    glGenTextures(1, &renderer->fonts[id].texture);
    glBindTexture(GL_TEXTURE_2D, renderer->fonts[id].texture);
    
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(png_name, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else {
        printf("Failed to load texture\n");
    }
    stbi_image_free(data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    
    renderer->fonts[id].texture_width = (float)width;
    renderer->fonts[id].texture_height = (float)height;
    
    // CSV Loading
    char* csv_name = arena_alloc(&renderer->arena, sizeof("assets/")-1 + strlen(name) + sizeof(".csv")-1 + 1);
    i = 0;
    memcpy(&csv_name[i], "assets/", sizeof("assets/")-1); i += sizeof("assets/")-1;
    memcpy(&csv_name[i], name, strlen(name)); i += strlen(name);
    memcpy(&csv_name[i], ".csv", sizeof(".csv")-1); i += sizeof(".csv")-1;
    csv_name[i] = '\0';
    
    FILE* csv_file = fopen(csv_name, "r");
    Rift_UIFont* curr_font = &renderer->fonts[id];
    curr_font->glyph_cap = 64;
    curr_font->glyphs = malloc(sizeof(Rift_UIFontGlyph) * 64);
    
    while (!feof(csv_file)) {
        if (curr_font->glyph_count + 1 >= curr_font->glyph_cap) {
            curr_font->glyph_cap *= 2;
            curr_font->glyphs = realloc(curr_font->glyphs, sizeof(Rift_UIFontGlyph) * curr_font->glyph_cap);
        }
        
        Rift_UIFontGlyph* curr_glyph = &curr_font->glyphs[curr_font->glyph_count++];
        fscanf(csv_file, "%d,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", &curr_glyph->id,
               &curr_glyph->advance, &curr_glyph->left,
               &curr_glyph->bottom, &curr_glyph->right,
               &curr_glyph->top, &curr_glyph->atlas_left,
               &curr_glyph->atlas_bottom, &curr_glyph->atlas_right,
               &curr_glyph->atlas_top);
    }
    
    printf("Glyphs Loaded in font %s: %d\n", name, curr_font->glyph_count);
    fclose(csv_file);
    
    return id;
}