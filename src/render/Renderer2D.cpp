//
// Renderer2D - 2D shader-based renderer (implementation).
//

#include "Renderer2D.h"

// Embedded shader sources so we don't have to ship .glsl files alongside
// the binary. The same shaders are in shaders/screen_quad.{vert,frag} for
// reference / editing; this string is the canonical copy used at runtime.
static const char *kVertexShaderSrc = R"(
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

static const char *kFragmentShaderSrc = R"(
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

Renderer2D &Renderer2D::get() {
  static Renderer2D instance;
  return instance;
}

bool Renderer2D::init(int windowW, int windowH) {
  if (!m_ready) {
    if (!m_shader.loadFromSource(kVertexShaderSrc, kFragmentShaderSrc)) {
      return false;
    }
    m_ready = true;
  }
  setViewport(windowW, windowH);
  return true;
}

void Renderer2D::setViewport(int windowW, int windowH) {
  // Orthographic projection: (0,0) at top-left, (w,h) at bottom-right.
  // This matches the legacy renderer's coordinate system (glOrtho with
  // top-left origin), so 2D draw calls don't need coordinate changes.
  m_projection = glm::ortho(0.0f, float(windowW), float(windowH), 0.0f,
                            -1.0f, 1.0f);
}

void Renderer2D::ensureWhiteTexture() {
  if (m_whiteTex != 0) return;
  glGenTextures(1, &m_whiteTex);
  glBindTexture(GL_TEXTURE_2D, m_whiteTex);
  // 1x1 RGBA white pixel.
  unsigned char white[4] = {255, 255, 255, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               white);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer2D::useShader() {
  if (!m_ready) return;
  m_shader.use();
  m_shader.setUniform("uProjection", m_projection);
  // Default modelview = identity (screen-space coordinates).
  // drawTexturedQuadMV / drawSolidQuadMV override this with the current
  // GL modelview matrix.
  m_shader.setUniform("uModelview", glm::mat4(1.0f));
  // Texture unit 0 is the default; the caller is responsible for
  // glBindTexture(GL_TEXTURE_2D, ...) before drawing.
  m_shader.setUniform("uTexture", 0);
}

void Renderer2D::unuseShader() {
  Shader::unuse();
}

glm::mat4 Renderer2D::currentModelview() const {
  double m[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, m);
  return glm::make_mat4(m);
}

glm::mat4 Renderer2D::currentProjection() const {
  double m[16];
  glGetDoublev(GL_PROJECTION_MATRIX, m);
  return glm::make_mat4(m);
}

void Renderer2D::drawTexturedQuad(float x, float y, float w, float h,
                                   const glm::vec4 &color) {
  drawTexturedQuadUV(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, color);
}

void Renderer2D::drawTexturedQuadUV(float x, float y, float w, float h,
                                     float u0, float v0, float u1, float v1,
                                     const glm::vec4 &color) {
  if (!m_ready) return;
  m_quad.setColor(color);
  m_quad.uploadQuad(x, y, x + w, y + h, u0, v0, u1, v1);
  m_quad.bind();
  m_quad.draw();
  m_quad.unbind();
}

void Renderer2D::drawSolidQuad(float x, float y, float w, float h,
                                const glm::vec4 &color) {
  if (!m_ready) return;
  ensureWhiteTexture();
  // The shader samples uTexture, so we must bind something. The 1x1 white
  // texture makes the sample contribute (1,1,1,1), leaving the result
  // equal to the per-vertex color.
  glBindTexture(GL_TEXTURE_2D, m_whiteTex);
  drawTexturedQuad(x, y, w, h, color);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer2D::drawTexturedQuadMV(float x, float y, float w, float h,
                                     const glm::vec4 &color) {
  if (!m_ready) return;
  // Override uModelview with the current GL modelview matrix so the quad's
  // local-space coordinates are transformed by the caller's glTranslated.
  m_shader.setUniform("uModelview", currentModelview());
  drawTexturedQuad(x, y, w, h, color);
  // Restore identity for any subsequent non-MV draw calls.
  m_shader.setUniform("uModelview", glm::mat4(1.0f));
}

void Renderer2D::drawSolidQuadMV(float x, float y, float w, float h,
                                  const glm::vec4 &color) {
  if (!m_ready) return;
  ensureWhiteTexture();
  glBindTexture(GL_TEXTURE_2D, m_whiteTex);
  m_shader.setUniform("uModelview", currentModelview());
  drawTexturedQuad(x, y, w, h, color);
  m_shader.setUniform("uModelview", glm::mat4(1.0f));
  glBindTexture(GL_TEXTURE_2D, 0);
}
