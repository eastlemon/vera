// Test fragment shader - samples a texture and multiplies by the per-vertex
// color. When the texture is a 1x1 white pixel, this acts as a solid-color
// shader; otherwise it's a textured quad with optional tint.

#version 330 core

in vec2 vUV;
in vec4 vColor;

uniform sampler2D uTexture;

out vec4 FragColor;

void main() {
  vec4 tex = texture(uTexture, vUV);
  FragColor = tex * vColor;
}
