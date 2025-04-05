#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in mat4 modelMatrix;

// uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * modelMatrix * vec4(position, 1.0f);
}
