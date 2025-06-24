#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec2 trim_coord;
const float pi2 = 2.0 * 3.14159265359;

void main() {
  gl_Position = projection * view * model * vec4(position, 1.0f);
  trim_coord = vec2(uv.x, uv.y) / pi2;
}
