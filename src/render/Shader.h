//
// Shader - GLSL shader compile/link/use helper.
//
// This is the foundation of the modern OpenGL renderer (Stage B of the
// fixed-function -> core-profile migration). It is intentionally minimal:
//   - load from source string or file
//   - compile vertex + fragment into a linked program
//   - uniform setters (int / float / vec2/3/4 / mat3 / mat4)
//   - attribute binding by name (for VAO setup)
//
// Nothing here depends on the existing fixed-function renderer, so this file
// can be built and unit-tested without touching any gameplay code.
//

#pragma once

#include "GLCompat.h"
#include <glm/glm.hpp>
#include <map>
#include <string>

class Shader {
public:
  Shader();
  ~Shader();

  // Disable copies - we own a GL program name.
  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  // Load+compile+link from source strings. Returns true on success.
  // On failure, the error log is printed to stderr and the program id stays 0.
  bool loadFromSource(const std::string &vertexSrc,
                      const std::string &fragmentSrc);

  // Convenience: same, but reads from .glsl files on disk.
  bool loadFromFile(const std::string &vertexPath,
                    const std::string &fragmentPath);

  // Bind this program for the next draw call. No-op if load failed.
  void use() const;

  // Unbind (bind program 0 = fixed-function pipeline).
  static void unuse();

  // True if the program is linked and ready.
  bool isValid() const { return m_program != 0; }

  // Direct GL program name. Use when you need raw GL access.
  GLuint programId() const { return m_program; }

  // ----- Uniform setters -----
  // Look up the uniform by name; -1 means "not found / optimised out".
  GLint uniformLocation(const char *name) const;

  void setUniform(const char *name, int value) const;
  void setUniform(const char *name, float value) const;
  void setUniform(const char *name, const glm::vec2 &v) const;
  void setUniform(const char *name, const glm::vec3 &v) const;
  void setUniform(const char *name, const glm::vec4 &v) const;
  void setUniform(const char *name, const glm::mat3 &m) const;
  void setUniform(const char *name, const glm::mat4 &m) const;

  // ----- Attribute location -----
  GLint attribLocation(const char *name) const;

  // Fetch the info log from the last compile/link operation.
  // Empty if no errors.
  std::string infoLog() const;

private:
  // Compile a single shader stage. Returns the shader id, or 0 on failure.
  static GLuint compileStage(GLenum type, const std::string &src);

  GLuint m_program = 0;
  // Cache uniform locations so we don't query the GL every frame.
  mutable std::map<std::string, GLint> m_uniformCache;
};
