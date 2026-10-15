//
// TextRenderer2D - shader-based glyph atlas text renderer (implementation).
//

#include "TextRenderer2D.h"
#include <cstdio>
#include <cstring>
#include <vector>

// ----- Embedded shaders -----
//
// The text shader is a variant of the 2D quad shader that takes a
// modelview matrix (so it respects the caller's glPushMatrix/glTranslated
// state) in addition to the orthographic projection.
//
// Vertex layout (8 floats per vertex, interleaved):
//   x, y       - position in modelview space (pixels)
//   u, v       - texture coordinate in atlas (0..1)
//   r, g, b, a - per-vertex color (allows gradient text in the future;
//                currently the whole string uses one color)

static const char *kTextVertexShader = R"(
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

uniform mat4 uProjection;
uniform mat4 uModelview;

out vec2 vUV;
out vec4 vColor;

void main() {
  gl_Position = uProjection * uModelview * vec4(aPos, 0.0, 1.0);
  vUV = aUV;
  vColor = aColor;
}
)";

static const char *kTextFragmentShader = R"(
#version 330 core

in vec2 vUV;
in vec4 vColor;

uniform sampler2D uTexture;

out vec4 FragColor;

void main() {
  vec4 tex = texture(uTexture, vUV);
  FragColor = tex * vColor;
}
)";

// Helper: compile a shader stage, return 0 on failure.
static GLuint compileStage(GLenum type, const char *src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);
  GLint ok = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (ok != GL_TRUE) {
    GLint logLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
    std::vector<char> log(logLen + 1, 0);
    glGetShaderInfoLog(shader, logLen, nullptr, log.data());
    std::fprintf(stderr, "TextRenderer2D: shader compile error:\n%s\n",
                 log.data());
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

TextRenderer2D::TextRenderer2D() {}

TextRenderer2D::~TextRenderer2D() {
  if (m_vbo) glDeleteBuffers(1, &m_vbo);
  if (m_vao) glDeleteVertexArrays(1, &m_vao);
  if (m_shader) glDeleteProgram(m_shader);
}

bool TextRenderer2D::init() {
  if (m_ready) return true;

  // Compile shaders.
  GLuint vs = compileStage(GL_VERTEX_SHADER, kTextVertexShader);
  if (!vs) return false;
  GLuint fs = compileStage(GL_FRAGMENT_SHADER, kTextFragmentShader);
  if (!fs) {
    glDeleteShader(vs);
    return false;
  }

  m_shader = glCreateProgram();
  glAttachShader(m_shader, vs);
  glAttachShader(m_shader, fs);
  glLinkProgram(m_shader);
  glDetachShader(m_shader, vs);
  glDetachShader(m_shader, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);

  GLint ok = GL_FALSE;
  glGetProgramiv(m_shader, GL_LINK_STATUS, &ok);
  if (ok != GL_TRUE) {
    GLint logLen = 0;
    glGetProgramiv(m_shader, GL_INFO_LOG_LENGTH, &logLen);
    std::vector<char> log(logLen + 1, 0);
    glGetProgramInfoLog(m_shader, logLen, nullptr, log.data());
    std::fprintf(stderr, "TextRenderer2D: link error:\n%s\n", log.data());
    glDeleteProgram(m_shader);
    m_shader = 0;
    return false;
  }

  m_locProjection = glGetUniformLocation(m_shader, "uProjection");
  m_locModelview = glGetUniformLocation(m_shader, "uModelview");
  m_locTexture = glGetUniformLocation(m_shader, "uTexture");

  // Create VAO + VBO.
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  // Allocate space for MAX_VERTS vertices, 8 floats each.
  glBufferData(GL_ARRAY_BUFFER, MAX_VERTS * 8 * sizeof(float), nullptr,
               GL_DYNAMIC_DRAW);

  // Attribute 0: position (vec2)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (const void *)0);
  // Attribute 1: uv (vec2)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (const void *)(2 * sizeof(float)));
  // Attribute 2: color (vec4)
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (const void *)(4 * sizeof(float)));

  glBindVertexArray(0);

  m_ready = true;
  return true;
}

glm::mat4 TextRenderer2D::currentModelview() const {
  double m[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, m);
  // glm uses column-major; OpenGL's matrix is also column-major, so the
  // memory layout matches directly.
  return glm::make_mat4(m);
}

glm::mat4 TextRenderer2D::currentProjection() const {
  double m[16];
  glGetDoublev(GL_PROJECTION_MATRIX, m);
  return glm::make_mat4(m);
}

void TextRenderer2D::drawText(const std::string &text, int maxChars,
                              float x, float y,
                              float glyphW, float glyphH,
                              float glyphOffsetY,
                              int atlasCols,
                              float atlasOriginX, float atlasOriginY,
                              float alignWidth, int align,
                              const glm::vec4 &color) {
  if (!m_ready && !init()) return;
  if (text.empty()) return;

  int len = std::min(maxChars, (int)text.length());
  if (len <= 0) return;

  // Compute alignment offset.
  float alignOffset = 0.0f;
  if (align == TEXT_ALIGN_RIGHT) {
    alignOffset = -alignWidth * float(len);
  } else if (align == TEXT_ALIGN_CENTER) {
    alignOffset = -(alignWidth * float(len)) * 0.5f;
  }

  // Build vertex buffer: 6 vertices per character (2 triangles).
  // Layout: pos.xy, uv.xy, color.rgba (8 floats per vertex).
  std::vector<float> verts(len * 6 * 8);

  // The atlas is 512x512. Glyph UVs are computed as:
  //   u0 = (atlasOriginX + col * glyphW) / 512
  //   v0 = (511 - (atlasOriginY + row * glyphH)) / 512   (flipped Y)
  //   u1 = u0 + glyphW / 512
  //   v1 = v0 - (glyphH + glyphOffsetY) / 512  (glyphOffsetY is negative
  //                                            for the big font's -4px gap)
  const float ATLAS_SIZE = 512.0f;

  int vi = 0;  // vertex index into verts[]
  float cursorX = x + alignOffset;

  for (int i = 0; i < len; ++i) {
    unsigned char c = (unsigned char)text[i];
    c = toupper(c);
    c -= ' ';
    int col = c % atlasCols;
    int row = c / atlasCols;

    // UV coordinates in the atlas.
    float u0 = (atlasOriginX + float(col) * glyphW) / ATLAS_SIZE;
    float u1 = (atlasOriginX + float(col) * glyphW + glyphW) / ATLAS_SIZE;
    // Atlas is bottom-up: row 0 is at the top of the atlas origin.
    float vTop = (ATLAS_SIZE - 1.0f - (atlasOriginY + float(row) * glyphH)) /
                 ATLAS_SIZE;
    float vBot = (ATLAS_SIZE - 1.0f -
                  (atlasOriginY + float(row) * glyphH + glyphH + glyphOffsetY)) /
                 ATLAS_SIZE;

    // Quad corners in local space.
    // Legacy code: glVertex3f(0, glyphH + glyphOffsetY, 0) for top-left,
    //              glVertex3f(0, 0, 0) for bottom-left,
    //              glVertex3f(glyphW, glyphH + glyphOffsetY, 0) for top-right,
    //              glVertex3f(glyphW, 0, 0) for bottom-right.
    // So the quad spans from y=0 to y=glyphH+glyphOffsetY vertically.
    float x0 = cursorX;
    float x1 = cursorX + glyphW;
    float y0 = y;
    float y1 = y + glyphH + glyphOffsetY;

    // Triangle 1: (x0,y0), (x0,y1), (x1,y0)
    // pos
    verts[vi++] = x0; verts[vi++] = y0;
    // uv
    verts[vi++] = u0; verts[vi++] = vBot;
    // color
    verts[vi++] = color.r; verts[vi++] = color.g;
    verts[vi++] = color.b; verts[vi++] = color.a;

    verts[vi++] = x0; verts[vi++] = y1;
    verts[vi++] = u0; verts[vi++] = vTop;
    verts[vi++] = color.r; verts[vi++] = color.g;
    verts[vi++] = color.b; verts[vi++] = color.a;

    verts[vi++] = x1; verts[vi++] = y0;
    verts[vi++] = u1; verts[vi++] = vBot;
    verts[vi++] = color.r; verts[vi++] = color.g;
    verts[vi++] = color.b; verts[vi++] = color.a;

    // Triangle 2: (x1,y0), (x0,y1), (x1,y1)
    verts[vi++] = x1; verts[vi++] = y0;
    verts[vi++] = u1; verts[vi++] = vBot;
    verts[vi++] = color.r; verts[vi++] = color.g;
    verts[vi++] = color.b; verts[vi++] = color.a;

    verts[vi++] = x0; verts[vi++] = y1;
    verts[vi++] = u0; verts[vi++] = vTop;
    verts[vi++] = color.r; verts[vi++] = color.g;
    verts[vi++] = color.b; verts[vi++] = color.a;

    verts[vi++] = x1; verts[vi++] = y1;
    verts[vi++] = u1; verts[vi++] = vTop;
    verts[vi++] = color.r; verts[vi++] = color.g;
    verts[vi++] = color.b; verts[vi++] = color.a;

    cursorX += glyphW;
  }

  // Upload + draw.
  glUseProgram(m_shader);

  // Pass current GL matrices so the shader respects the caller's
  // glPushMatrix/glTranslated state.
  glUniformMatrix4fv(m_locProjection, 1, GL_FALSE,
                     glm::value_ptr(currentProjection()));
  glUniformMatrix4fv(m_locModelview, 1, GL_FALSE,
                     glm::value_ptr(currentModelview()));
  glUniform1i(m_locTexture, 0);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vi * sizeof(float), verts.data());
  glDrawArrays(GL_TRIANGLES, 0, len * 6);
  glBindVertexArray(0);

  glUseProgram(0);
}
