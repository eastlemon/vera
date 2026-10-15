# src/render/ — rendering layer

The renderer is in the middle of a migration from fixed-function OpenGL
(`glBegin/glEnd`, display lists, `glLight*`, `glFog*`) to a modern
shader-based pipeline (GLSL 3.3 core profile + VBO/VAO).

## Files

| File | Status | Description |
|------|--------|-------------|
| `OpenGLHelpers.{h,cpp}` | Legacy (will be removed) | Fixed-function GL helpers: color, multitexture, primitives, display lists |
| `OpenGLExtFunctions.h` | Legacy (will be removed) | ARB multitexture function-pointer typedefs |
| `BTextures.{h,cpp}` | Active | TGA loader + texture manager (texture IDs are still GL names; works in both pipelines) |
| `BCamera.{h,cpp}` | Active | Camera location / orientation / mode |
| `BTextRenderer.{h,cpp}` | Active (will need rewrite) | Glyph-atlas text rendering via `glBegin/glEnd` |

## New (Stage A — shader framework foundation)

| File | Description |
|------|-------------|
| `Shader.{h,cpp}` | GLSL shader compile/link/use helper. Loads from source string or file, caches uniform locations, exposes typed setters (`int` / `float` / `vec2/3/4` / `mat3/4`). |
| `Mesh2D.{h,cpp}` | VBO+VAO for screen-aligned quads. Vertex layout: `pos.xy + uv.xy + color.rgba` (8 floats). 4 vertices + 6 indices. Used by Stage B (2D UI rendering). |

## New (Stage B — 2D renderer, in progress)

| File | Description |
|------|-------------|
| `Renderer2D.{h,cpp}` | Singleton that owns the 2D shader + fullscreen quad mesh. Entry point for all 2D rendering: `drawTexturedQuad(x,y,w,h)`, `drawTexturedQuadUV(...)` for sprite atlases, `drawSolidQuad(x,y,w,h,color)` for solid colors (uses an internal 1x1 white texture). Embeds the shader sources so no external `.glsl` files are needed at runtime. |

## Test shaders

`../../shaders/screen_quad.{vert,frag}` — a minimal shader pair for the
2D renderer. The vertex shader takes `aPos` (in pixels), `aUV`, `aColor`
and projects via `uProjection` (an orthographic matrix). The fragment
shader samples `uTexture` and multiplies by `vColor`. This is enough to
render every 2D element in the game (menu, HUD, dialogs) — when the
texture is a 1x1 white pixel it acts as a solid-color shader.

## Migration plan

| Stage | Scope | Status |
|-------|-------|--------|
| **A** | Shader + Mesh2D + GLM + GLEW — foundation only, no game code changes | ✅ Done |
| **B** | 2D rendering: menu background (first), then HUD, dialogs, dashboard, NavSat, service window | 🔄 In progress — menu background migrated to Renderer2D |
| **C** | 3D rendering: terrain, objects, vehicle, sky, water, shadows | Pending |
| **D** | Cleanup: delete legacy `OpenGLHelpers`, `OpenGLExtFunctions`, switch to core profile | Pending |

## GL function loading

The codebase uses **GLEW** (`brew install glew` on macOS, `apt install
libglew-dev` on Debian/Ubuntu) to load modern GL function pointers
(`glGenVertexArrays`, `glCreateShader`, `glUniformMatrix4fv`, etc.).
GLEW was chosen over GLAD because it also exposes the deprecated
fixed-function functions (`glBegin`, `glEnd`, `glVertex3f`, ...) that
the legacy renderer still uses, so the migration can be incremental.

Stage D will remove the legacy code path and we can switch to a
stricter loader (or just use GLEW's core-profile-only mode).
