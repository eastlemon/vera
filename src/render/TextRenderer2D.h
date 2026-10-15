//
// TextRenderer2D - shader-based glyph atlas text renderer.
//
// Replaces BTextRenderer's glBegin/glEnd-per-character loop with a single
// VBO upload + one glDrawArrays call per text string.
//
// The glyph atlas is NavSatWndBigLetters.tga (512x512 RGBA), which contains
// two font zones:
//   - Big font:   14.5 x 28.75 px, 5 columns, origin (1, 0)
//   - Small font:  8.25 x 13.9 px, 10 columns, origin (401, 284)
//
// Characters are indexed by (char - ' '), uppercased. The atlas is laid out
// bottom-up (TGA origin), so the V coordinate is flipped relative to the
// screen-space Y.
//
// Coordinate system: this renderer respects the current GL modelview matrix
// (so callers can still use glPushMatrix/glTranslated/glPopMatrix to position
// text). On each draw, the modelview matrix is read back via glGetDoublev
// and passed to the shader as a uniform. This keeps the API compatible with
// all existing callers (BUISelectionList, BServiceWnd, BNavSatWnd, etc.)
// without rewriting their coordinate logic.
//

#pragma once

#include "GLCompat.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

// Text alignment enum (0=left, 1=right, 2=center).
// Matches BTextRenderer::TTextAlign values.
enum TextAlign { TEXT_ALIGN_LEFT = 0, TEXT_ALIGN_RIGHT = 1, TEXT_ALIGN_CENTER = 2 };

class TextRenderer2D {
public:
  // Maximum characters per draw call. 1024 is more than enough for any
  // single text string in the game (the longest is ~60 chars).
  static const int MAX_CHARS = 1024;
  static const int VERTS_PER_CHAR = 6;  // 2 triangles = 6 vertices
  static const int MAX_VERTS = MAX_CHARS * VERTS_PER_CHAR;

  TextRenderer2D();
  ~TextRenderer2D();

  TextRenderer2D(const TextRenderer2D &) = delete;
  TextRenderer2D &operator=(const TextRenderer2D &) = delete;

  // Compile the text shader + create VAO/VBO. Idempotent.
  // Returns true on success.
  bool init();

  // Draw a text string using the big font (14.5x28.75) from the glyph atlas.
  // (x, y) is the baseline anchor in the current modelview coordinate space.
  // The caller must have bound the glyph atlas texture (BTextures::PANEL)
  // and called glEnable(GL_TEXTURE_2D) before calling this.
  void drawBigText(const std::string &text, float x, float y,
                   TextAlign align, const glm::vec4 &color);

  // Draw a text string using the small font (8.25x13.9) from the glyph atlas.
  void drawSmallText(const std::string &text, int maxChars, float x, float y,
                     TextAlign align, const glm::vec4 &color);

  // Draw a text string with explicit glyph atlas parameters.
  // Used internally by drawBigText/drawSmallText; exposed for custom fonts.
  void drawText(const std::string &text, int maxChars,
                float x, float y,
                float glyphW, float glyphH,
                float glyphOffsetY,  // pixels to subtract from glyphH for the
                                     // visible top (big font uses -4.0)
                int atlasCols,       // glyphs per row in the atlas
                float atlasOriginX,  // pixel offset of the first glyph
                float atlasOriginY,  //   in the atlas texture
                float alignWidth,    // glyph advance width for alignment
                int align,           // 0=left, 1=right, 2=center
                const glm::vec4 &color);

private:
  bool m_ready = false;
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  GLuint m_shader = 0;
  GLint m_locProjection = -1;
  GLint m_locModelview = -1;
  GLint m_locTexture = -1;

  // Read the current modelview matrix from the GL and return it as glm::mat4.
  // Used so that callers' glPushMatrix/glTranslated calls still work.
  glm::mat4 currentModelview() const;

  // Read the current projection matrix (set up by Setup2DRendering).
  glm::mat4 currentProjection() const;
};


