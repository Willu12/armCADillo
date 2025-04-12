#version 460
layout(lines_adjacency) in;
layout(line_strip, max_vertices = 128) out;

uniform int uNumSegments;
uniform mat4 view;
uniform mat4 projection;

vec4 bezier3(vec4 b0, vec4 b1, vec4 b2, vec4 b3, float t) {
  float t1 = 1.0f - t;

  b0 = t1 * b0 + t * b1;
  b1 = t1 * b1 + t * b2;
  b2 = t1 * b2 + t * b3;

  b0 = t1 * b0 + t * b1;
  b1 = t1 * b1 + t * b2;

  return t1 * b0 + t * b1;
}
void main() {
  vec4 b0 = gl_in[0].gl_Position;
  vec4 b1 = gl_in[1].gl_Position;
  vec4 b2 = gl_in[2].gl_Position;
  vec4 b3 = gl_in[3].gl_Position;

  mat4 pv = projection * view;

  for (int i = 0; i <= uNumSegments; ++i) {
    float t = float(i) / uNumSegments;
    vec4 point = bezier3(b0, b1, b2, b3, t);
    gl_Position = pv * point;
    EmitVertex();
  }

  EndPrimitive();
}
