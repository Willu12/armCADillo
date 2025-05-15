#version 430

uniform vec2 startPos;
uniform vec2 endPos;

vec3 getPos(int i) {
  if (i == 0)
    return vec3(startPos.x, startPos.y, 0.0);
  if (i == 1)
    return vec3(startPos.x, endPos.y, 0.0);
  if (i == 2)
    return vec3(endPos.x, endPos.y, 1.0);
  if (i == 3)
    return vec3(endPos.x, startPos.y, 0.0);
  return vec3(0.0);
}

int indices[6] = int[6](0, 2, 1, 2, 0, 3);

void main() {
  int index = indices[gl_VertexID];
  vec3 pos3 = getPos(index);
  gl_Position = vec4(pos3, 1.0);
}
