#version 460
layout(lines_adjacency) in;
layout(line_strip, max_vertices = 128) out;

uniform int uNumSegments;

vec3 bezier3(vec3 b0, vec3 b1, vec3 b2, vec3 b3, float t) {
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

  for (int i = 0; i <= uNumSegments; ++i) {
    float t = float(i) / uNumSegments;
    float u = 1.0 - t;

    vec4 point = u * u * u * b0 + 3 * u * u * t * b1 + 3 * u * t * t * b2 +
                 t * t * t * b3;

    EmitVertex();
  }

  EndPrimitive();
}
