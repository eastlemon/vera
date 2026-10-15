//
// Mesh2D - 2D textured quad helper (implementation).
//

#include "Mesh2D.h"

// Vertex layout: 8 floats per vertex = pos.xy + uv.xy + color.rgba
struct Vertex2D {
  float x, y;     // position (pixels)
  float u, v;     // texcoord (0..1)
  float r, g, b, a; // color
};

static const GLuint kIndices[6] = {0, 1, 2, 2, 1, 3};

Mesh2D::Mesh2D() {
  // VAO must be created first.
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  // Attribute 0: position (vec2)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
                        (const void *)offsetof(Vertex2D, x));

  // Attribute 1: uv (vec2)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
                        (const void *)offsetof(Vertex2D, u));

  // Attribute 2: color (vec4)
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
                        (const void *)offsetof(Vertex2D, r));

  glGenBuffers(1, &m_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices,
               GL_STATIC_DRAW);

  // Allocate VBO space for 4 vertices (will be overwritten by upload*).
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * 4, nullptr, GL_DYNAMIC_DRAW);

  glBindVertexArray(0);
}

Mesh2D::~Mesh2D() {
  if (m_ibo) glDeleteBuffers(1, &m_ibo);
  if (m_vbo) glDeleteBuffers(1, &m_vbo);
  if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Mesh2D::setColor(const glm::vec4 &color) { m_color = color; }

void Mesh2D::uploadQuad(float x0, float y0, float x1, float y1,
                        float u0, float v0, float u1, float v1) {
  // In screen-space coordinates Y increases downward (y0 = top, y1 = bottom).
  // OpenGL texture V=0 is the bottom of the texture, V=1 is the top.
  // To display textures right-side-up, we swap v0/v1 so that:
  //   top of screen (y0) → v1 (top of texture)
  //   bottom of screen (y1) → v0 (bottom of texture)
  Vertex2D verts[4] = {
    { x0, y0, u0, v1, m_color.r, m_color.g, m_color.b, m_color.a }, // 0: top-left
    { x0, y1, u0, v0, m_color.r, m_color.g, m_color.b, m_color.a }, // 1: bottom-left
    { x1, y0, u1, v1, m_color.r, m_color.g, m_color.b, m_color.a }, // 2: top-right
    { x1, y1, u1, v0, m_color.r, m_color.g, m_color.b, m_color.a }, // 3: bottom-right
  };

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh2D::uploadScreenQuad(float w, float h) {
  uploadQuad(0.0f, 0.0f, w, h, 0.0f, 0.0f, 1.0f, 1.0f);
}

void Mesh2D::bind() const {
  glBindVertexArray(m_vao);
}

void Mesh2D::draw() const {
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Mesh2D::unbind() const {
  glBindVertexArray(0);
}
