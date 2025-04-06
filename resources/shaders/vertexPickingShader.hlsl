#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in mat4 modelMatrix;
layout(location = 5) in uint vObjectIndex;

uniform mat4 view;
uniform mat4 projection;

// layout(std430, binding = 0) buffer ObjectIndices { uint objectIndices[]; };
flat out uint fObjectIndex; // <- Pass to fragment shader without interpolation

void main() {
  fObjectIndex = vObjectIndex;
  gl_Position = projection * view * modelMatrix * vec4(position, 1.0f);
}
