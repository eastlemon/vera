// Test vertex shader for the modern renderer (GLSL 1.20 for macOS compat).
//
// Stage B of the GL modernisation uses this as the basis for all 2D
// rendering (menu, HUD, dialogs). It expects:
//   attribute 0: vec2 aPos   (screen pixel coordinates)
//   attribute 1: vec2 aUV    (texture coordinates, 0..1)
//   attribute 2: vec4 aColor (per-vertex tint, 0..1)
//
// Uniforms:
//   uProjection : mat4 ortho projection (pixels -> clip space)
//   uModelview  : mat4 modelview (for glPushMatrix/glTranslated compat)
//   uTexture    : sampler2D bound to texture unit 0

#version 120

attribute vec2 aPos;
attribute vec2 aUV;
attribute vec4 aColor;

uniform mat4 uProjection;
uniform mat4 uModelview;

varying vec2 vUV;
varying vec4 vColor;

void main() {
  gl_Position = uProjection * uModelview * vec4(aPos, 0.0, 1.0);
  vUV = aUV;
  vColor = aColor;
}
