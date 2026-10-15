//
// Renderer2D - singleton that owns the shader + mesh for 2D rendering.
//
// This is the entry point for Stage B of the OpenGL modernisation.
// All 2D rendering (menu background, HUD, dialogs, dashboard) will go
// through this class instead of using glBegin/glEnd directly.
//
// Usage:
//   if (Renderer2D::get().init(windowWidth, windowHeight)) {
//     Renderer2D::get().drawTexturedQuad(x, y, w, h, textureId, color);
//   }
//
// The init() call is idempotent - it only compiles the shader and uploads
// the fullscreen quad the first time. Subsequent calls just update the
// projection matrix (in case the window was resized).
//

#pragma once

#include "Mesh2D.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Renderer2D {
public:
  // Singleton accessor.
  static Renderer2D &get();

  // Compile the shader + upload the fullscreen quad.
  // Returns true on success. Safe to call multiple times - subsequent
  // calls only update the projection matrix.
  // windowW/windowH are the pixel dimensions of the output window; the
  // projection maps (0,0) to top-left and (w,h) to bottom-right, matching
  // the legacy renderer's coordinate system.
  bool init(int windowW, int windowH);

  // True if init() succeeded and the renderer is ready to draw.
  bool isReady() const { return m_ready; }

  // Update the orthographic projection. Call after a window resize.
  void setViewport(int windowW, int windowH);

  // ----- Drawing primitives -----

  // Draw a textured quad covering (x, y) to (x+w, y+h).
  // The currently-bound GL texture (glBindTexture) is sampled.
  // color is an RGBA multiplier (default white = no tint).
  void drawTexturedQuad(float x, float y, float w, float h,
                        const glm::vec4 &color = glm::vec4(1.0f));

  // Draw a textured quad with explicit UV sub-region (for sprite atlases).
  void drawTexturedQuadUV(float x, float y, float w, float h,
                          float u0, float v0, float u1, float v1,
                          const glm::vec4 &color = glm::vec4(1.0f));

  // Draw a solid-color quad (no texture). Implemented as a textured quad
  // with a 1x1 white texture internally; if no white texture was created
  // yet, falls back to drawing with GL_TEXTURE_2D disabled.
  void drawSolidQuad(float x, float y, float w, float h,
                     const glm::vec4 &color);

  // ----- Modelview-aware variants -----
  // These read the current GL modelview matrix (set up by the caller via
  // glPushMatrix / glTranslated / glPopMatrix) and pass it to the shader.
  // Use these when the quad's coordinates are in a local space established
  // by a glTranslated call - e.g. inside BUISelectionList::DrawAt.
  void drawTexturedQuadMV(float x, float y, float w, float h,
                          const glm::vec4 &color = glm::vec4(1.0f));
  void drawSolidQuadMV(float x, float y, float w, float h,
                       const glm::vec4 &color);

  // Bind the 2D shader. Use this when you need to set custom uniforms
  // before calling drawTexturedQuad.
  void useShader();

  // Unbind the 2D shader (restore fixed-function pipeline).
  void unuseShader();

private:
  Renderer2D() = default;
  ~Renderer2D() = default;
  Renderer2D(const Renderer2D &) = delete;
  Renderer2D &operator=(const Renderer2D &) = delete;

  bool m_ready = false;
  Shader m_shader;
  Mesh2D m_quad;
  glm::mat4 m_projection;

  // 1x1 white texture for drawSolidQuad.
  GLuint m_whiteTex = 0;
  void ensureWhiteTexture();

  // Read the current GL modelview matrix and return it as glm::mat4.
  glm::mat4 currentModelview() const;
  // Read the current GL projection matrix.
  glm::mat4 currentProjection() const;
};
