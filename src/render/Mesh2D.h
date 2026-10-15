//
// Mesh2D - a 2D textured quad helper for the modern (shader-based) renderer.
//
// Stage B of the OpenGL modernisation will replace all glBegin/glEnd pairs
// that draw screen-aligned quads (menu, HUD, dialogs, dashboard) with
// instances of this class.
//
// Layout (4 vertices, 6 indices = 2 triangles):
//   pos    = vec2 (screen coordinates, pixels)
//   uv     = vec2 (texture coordinates, 0..1)
//   color  = vec4 (RGBA, 0..1; defaults to white)
//
// Vertex order (counter-clockwise):
//   0 --- 1
//   |  \  |
//   |   \ |
//   2 --- 3
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/glew.h>
#endif
#include <glm/glm.hpp>

class Mesh2D {
public:
  Mesh2D();
  ~Mesh2D();

  // Disable copies - we own GL buffers.
  Mesh2D(const Mesh2D &) = delete;
  Mesh2D &operator=(const Mesh2D &) = delete;

  // Upload the standard fullscreen quad: 4 vertices covering (0,0)..(w,h).
  // Vertex positions are in pixels; the shader is expected to project them
  // via an orthographic matrix.
  void uploadScreenQuad(float w, float h);

  // Upload a generic quad with explicit UVs (for sprite sheets, sub-regions
  // of an atlas, etc.).
  //   (x0, y0) bottom-left in pixels, (x1, y1) top-right in pixels.
  //   (u0, v0) bottom-left in UV,    (u1, v1) top-right in UV.
  void uploadQuad(float x0, float y0, float x1, float y1,
                  float u0, float v0, float u1, float v1);

  // Per-vertex color (used when the shader multiplies the texture sample
  // by this color). Default is (1,1,1,1).
  void setColor(const glm::vec4 &color);

  // Bind VAO + enable vertex attributes 0 (pos), 1 (uv), 2 (color).
  // The caller is responsible for binding a Shader and a texture
  // before calling draw().
  void bind() const;

  // Draw the 2 triangles (6 indices). Assumes bind() has been called.
  void draw() const;

  // Disable vertex attribs and unbind VAO.
  void unbind() const;

private:
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLuint m_ibo = 0;
  glm::vec4 m_color = glm::vec4(1.0f);
};
