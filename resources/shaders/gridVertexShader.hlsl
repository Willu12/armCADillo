#version 330

out vec3 worldPos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraWorldPos;
uniform float gridSize = 100.0f;

const vec3 pos[4] = vec3[4](vec3(-1.0, 0.0, -1.0), vec3(1.0, 0.0, -1.0),
                            vec3(1.0, 0.0, 1.0), vec3(-1.0, 0.0, 1.0));

const int indices[6] = int[6](0, 2, 1, 2, 0, 3);

void main()
{
  int index = indices[gl_VertexID];

  vec3 pos3 = pos[index] * gridSize;
  pos3.x += cameraWorldPos.x;
  pos3.z += cameraWorldPos.z;

  vec4 vPos = vec4(pos3, 1.0);
  gl_Position = projection * view * vPos;

  worldPos = pos3;
}