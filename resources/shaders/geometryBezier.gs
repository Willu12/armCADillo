#version 460
layout(lines_adjacency) in;
layout(line_strip, max_vertices = 128) out;

uniform mat4 view;
uniform mat4 projection;
uniform bool renderPolyLine;
uniform int screenResolution;

int calculateLength(in vec4 b0, in vec4 b1, in vec4 b2, in vec4 b3,
                    in mat4 pv) {
  vec4 sb0 = pv * b0;
  vec4 sb1 = pv * b1;
  vec4 sb2 = pv * b2;
  vec4 sb3 = pv * b3;

  sb0 /= sb0.w;
  sb1 /= sb1.w;
  sb2 /= sb2.w;
  sb3 /= sb3.w;

  float length_screen = distance(sb0.xy, sb1.xy) + distance(sb1.xy, sb2.xy) +
                        distance(sb2.xy, sb3.xy);
  int segments = 10 * int(length_screen * screenResolution);
  return clamp(segments, 8, 128);
}

vec4 bezier3(vec4 b0, vec4 b1, vec4 b2, vec4 b3, float t) {
  float t1 = 1.0f - t;

  b0 = t1 * b0 + t * b1;
  b1 = t1 * b1 + t * b2;
  b2 = t1 * b2 + t * b3;

  b0 = t1 * b0 + t * b1;
  b1 = t1 * b1 + t * b2;

  return t1 * b0 + t * b1;
}

void createPolyLine(in vec4 b0, in vec4 b1, in vec4 b2, in vec4 b3,
                    in mat4 pv) {
  gl_Position = pv * b0;
  EmitVertex();
  gl_Position = pv * b1;
  EmitVertex();
  gl_Position = pv * b2;
  EmitVertex();
  gl_Position = pv * b3;
  EmitVertex();
  EndPrimitive();
}

void main() {
  vec4 b0 = gl_in[0].gl_Position;
  vec4 b1 = gl_in[1].gl_Position;
  vec4 b2 = gl_in[2].gl_Position;
  vec4 b3 = gl_in[3].gl_Position;

  mat4 pv = projection * view;

  if (renderPolyLine)
    createPolyLine(b0, b1, b2, b3, pv);

  int segmentsCount = calculateLength(b0, b1, b2, b3, pv);
  for (int i = 0; i <= segmentsCount; ++i) {
    float t = float(i) / segmentsCount;
    vec4 point = bezier3(b0, b1, b2, b3, t);
    gl_Position = pv * point;
    EmitVertex();
  }

  EndPrimitive();
}
