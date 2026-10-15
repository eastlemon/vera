// Test vertex shader for the modern renderer.
//
// Stage B of the GL modernisation will use this as the basis for all 2D
// rendering (menu, HUD, dialogs). It expects:
//   attribute 0: vec2 aPos   (screen pixel coordinates)
//   attribute 1: vec2 aUV    (texture coordinates, 0..1)
//   attribute 2: vec4 aColor (per-vertex tint, 0..1)
//
// Uniforms:
//   uProjection : mat4 ortho projection (pixels -> clip space)
//   uTexture    : sampler2D bound to texture unit 0
//
// The varying vColor is passed through to the fragment shader so the same
// shader works for both solid-color and textured quads (sample with a 1x1
// white texture for the solid-color case).

#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

uniform mat4 uProjection;

out vec2 vUV;
out vec4 vColor;

void main() {
  gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
  vUV = aUV;
  vColor = aColor;
}
