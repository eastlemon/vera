//
// GLCompat.h - cross-platform OpenGL header that provides modern GL 3.0+
// function declarations on all platforms.
//
// On Linux/Windows: includes GLEW, which loads all modern GL function
// pointers (glGenVertexArrays, glCreateShader, etc.).
//
// On macOS: the system OpenGL framework (OpenGL/gl.h + OpenGL/glext.h)
// declares VAO functions with the APPLE suffix (glGenVertexArraysAPPLE,
// glBindVertexArrayAPPLE, glDeleteVertexArraysAPPLE) instead of the
// standard names. This header maps the standard names to the APPLE
// variants via macros, so the rest of the codebase can use the standard
// names on all platforms.
//
// Include this header BEFORE any other GL header.
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__)
  // macOS: use the system OpenGL framework.
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>

  // Map standard GL 3.0 VAO function names to the APPLE-suffixed
  // versions declared in macOS glext.h. These are functionally
  // identical - Apple implemented VAOs as an extension before they
  // became core in GL 3.0, and the SDK never added the standard names.
  #ifndef glGenVertexArrays
    #define glGenVertexArrays(n, ids)        glGenVertexArraysAPPLE(n, ids)
  #endif
  #ifndef glDeleteVertexArrays
    #define glDeleteVertexArrays(n, ids)     glDeleteVertexArraysAPPLE(n, ids)
  #endif
  #ifndef glBindVertexArray
    #define glBindVertexArray(id)            glBindVertexArrayAPPLE(id)
  #endif

  // glDrawArrays, glDrawElements, glBufferData, glBufferSubData,
  // glBindBuffer, glGenBuffers, glDeleteBuffers,
  // glVertexAttribPointer, glEnableVertexAttribArray,
  // glDisableVertexAttribArray are all available in macOS gl.h /
  // glext.h under their standard names (they were GL 1.5 VBO API).
  // No remapping needed for those.

  // Shader functions (glCreateShader, glShaderSource, etc.) are also
  // available in glext.h under their standard names on macOS 10.7+.
  // If any are missing, they can be remapped here.
#else
  // Linux / Windows: use GLEW.
  #include <GL/glew.h>
#endif
