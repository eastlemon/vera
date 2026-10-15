# vendor/

Third-party header-only libraries used by Vera.

## glm/

[OpenGL Mathematics (GLM)](https://github.com/g-truc/glm) — header-only C++
math library that mimics GLSL's vector/matrix types and provides
`perspective`, `lookAt`, `translate`, `rotate`, `scale`, etc.

Used by `src/render/Shader.h` and `src/render/Mesh2D.h` for the modern
shader-based renderer. The legacy fixed-function code still uses
`gluPerspective`/`gluLookAt` directly; those will be replaced with GLM
in Stage B/C of the OpenGL modernisation.

### Update
```bash
# Download the latest release tarball from
# https://github.com/g-truc/glm/releases and unpack it here, keeping
# the structure:
#   vendor/glm/glm.hpp
#   vendor/glm/detail/
#   vendor/glm/gtc/
#   vendor/glm/gtx/
#   ...
```

Or on Debian/Ubuntu: `apt-get download libglm-dev && dpkg-deb -x libglm-dev*.deb /tmp/glm_extract && cp -r /tmp/glm_extract/usr/include/glm vendor/`
