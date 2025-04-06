#version 330

flat in uint fObjectIndex;

out uvec3 FragColor;

void main() { FragColor = uvec3(fObjectIndex, 0, gl_PrimitiveID); }