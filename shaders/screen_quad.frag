// Test fragment shader - samples a texture and multiplies by the per-vertex
// color. When the texture is a 1x1 white pixel, this acts as a solid-color
// shader; otherwise it's a textured quad with optional tint.
// GLSL 1.20 for macOS compatibility.

#version 120

varying vec2 vUV;
varying vec4 vColor;

uniform sampler2D uTexture;

void main() {
  vec4 tex = texture2D(uTexture, vUV);
  gl_FragColor = tex * vColor;
}
