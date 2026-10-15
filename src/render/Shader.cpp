//
// Shader - GLSL shader compile/link/use helper (implementation).
//

#include "Shader.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

Shader::Shader() = default;

Shader::~Shader() {
  if (m_program != 0) {
    glDeleteProgram(m_program);
  }
}

// ---------------------------------------------------------------------------
// Compilation
// ---------------------------------------------------------------------------

GLuint Shader::compileStage(GLenum type, const std::string &src) {
  GLuint shader = glCreateShader(type);
  const char *srcPtr = src.c_str();
  glShaderSource(shader, 1, &srcPtr, nullptr);
  glCompileShader(shader);

  GLint ok = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (ok != GL_TRUE) {
    GLint logLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
    std::vector<char> log(logLen + 1, 0);
    glGetShaderInfoLog(shader, logLen, nullptr, log.data());
    const char *typeName = (type == GL_VERTEX_SHADER)   ? "VERTEX"
                           : (type == GL_FRAGMENT_SHADER) ? "FRAGMENT"
                                                          : "UNKNOWN";
    std::fprintf(stderr, "Shader: %s compile error:\n%s\n", typeName, log.data());
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

// ---------------------------------------------------------------------------
// Linking
// ---------------------------------------------------------------------------

bool Shader::loadFromSource(const std::string &vertexSrc,
                            const std::string &fragmentSrc) {
  // Delete any previous program.
  if (m_program != 0) {
    glDeleteProgram(m_program);
    m_program = 0;
    m_uniformCache.clear();
  }

  GLuint vs = compileStage(GL_VERTEX_SHADER, vertexSrc);
  if (vs == 0) return false;
  GLuint fs = compileStage(GL_FRAGMENT_SHADER, fragmentSrc);
  if (fs == 0) {
    glDeleteShader(vs);
    return false;
  }

  m_program = glCreateProgram();
  glAttachShader(m_program, vs);
  glAttachShader(m_program, fs);

  // Bind attribute locations BEFORE linking. GLSL 1.20 (used for macOS
  // compatibility) does not support 'layout(location = N)' in the shader
  // source, so we must bind them here. Mesh2D's VAO uses hardcoded
  // locations 0 (pos), 1 (uv), 2 (color).
  glBindAttribLocation(m_program, 0, "aPos");
  glBindAttribLocation(m_program, 1, "aUV");
  glBindAttribLocation(m_program, 2, "aColor");

  glLinkProgram(m_program);

  // Shaders can be detached and deleted once linked.
  glDetachShader(m_program, vs);
  glDetachShader(m_program, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);

  GLint ok = GL_FALSE;
  glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
  if (ok != GL_TRUE) {
    GLint logLen = 0;
    glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
    std::vector<char> log(logLen + 1, 0);
    glGetProgramInfoLog(m_program, logLen, nullptr, log.data());
    std::fprintf(stderr, "Shader: link error:\n%s\n", log.data());
    glDeleteProgram(m_program);
    m_program = 0;
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// File loading
// ---------------------------------------------------------------------------

static bool readFile(const std::string &path, std::string &out) {
  std::ifstream f(path);
  if (!f) return false;
  std::stringstream ss;
  ss << f.rdbuf();
  out = ss.str();
  return true;
}

bool Shader::loadFromFile(const std::string &vertexPath,
                          const std::string &fragmentPath) {
  std::string vs, fs;
  if (!readFile(vertexPath, vs)) {
    std::fprintf(stderr, "Shader: cannot read vertex shader: %s\n",
                 vertexPath.c_str());
    return false;
  }
  if (!readFile(fragmentPath, fs)) {
    std::fprintf(stderr, "Shader: cannot read fragment shader: %s\n",
                 fragmentPath.c_str());
    return false;
  }
  return loadFromSource(vs, fs);
}

// ---------------------------------------------------------------------------
// Binding
// ---------------------------------------------------------------------------

void Shader::use() const {
  if (m_program != 0) {
    glUseProgram(m_program);
  }
}

void Shader::unuse() { glUseProgram(0); }

// ---------------------------------------------------------------------------
// Uniforms
// ---------------------------------------------------------------------------

GLint Shader::uniformLocation(const char *name) const {
  auto it = m_uniformCache.find(name);
  if (it != m_uniformCache.end()) {
    return it->second;
  }
  GLint loc = glGetUniformLocation(m_program, name);
  m_uniformCache[name] = loc;
  return loc;
}

void Shader::setUniform(const char *name, int value) const {
  GLint loc = uniformLocation(name);
  if (loc != -1) glUniform1i(loc, value);
}

void Shader::setUniform(const char *name, float value) const {
  GLint loc = uniformLocation(name);
  if (loc != -1) glUniform1f(loc, value);
}

void Shader::setUniform(const char *name, const glm::vec2 &v) const {
  GLint loc = uniformLocation(name);
  if (loc != -1) glUniform2f(loc, v.x, v.y);
}

void Shader::setUniform(const char *name, const glm::vec3 &v) const {
  GLint loc = uniformLocation(name);
  if (loc != -1) glUniform3f(loc, v.x, v.y, v.z);
}

void Shader::setUniform(const char *name, const glm::vec4 &v) const {
  GLint loc = uniformLocation(name);
  if (loc != -1) glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void Shader::setUniform(const char *name, const glm::mat3 &m) const {
  GLint loc = uniformLocation(name);
  if (loc != -1) glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
}

void Shader::setUniform(const char *name, const glm::mat4 &m) const {
  GLint loc = uniformLocation(name);
  if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
}

GLint Shader::attribLocation(const char *name) const {
  return glGetAttribLocation(m_program, name);
}

std::string Shader::infoLog() const {
  if (m_program == 0) return "";
  GLint logLen = 0;
  glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
  if (logLen <= 0) return "";
  std::vector<char> log(logLen + 1, 0);
  glGetProgramInfoLog(m_program, logLen, nullptr, log.data());
  return std::string(log.data());
}
