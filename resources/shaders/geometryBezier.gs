#version 460
layout(points) in;
layout(line_strip, max_vertices = 256) out;

uniform mat4 view;
uniform mat4 projection;
uniform bool renderPolyLine;
uniform int screenResolution;

in VS_OUT {
  int len;
  vec4 pts[4];
}
gs_in[];

int calculateLength(in vec4 b0, in vec4 b1, in vec4 b2, in vec4 b3, in mat4 pv,
                    in int pointCount) {
  vec4 sb0 = pv * b0;
  sb0 /= sb0.w;
  vec4 sb1 = pv * b1;
  sb1 /= sb1.w;
  vec4 sb2 = pv * b2;
  sb2 /= sb2.w;
  vec4 sb3 = pv * b3;
  sb3 /= sb3.w;

  float length_screen;

  if (pointCount == 3) {
    length_screen = distance(sb0.xy, sb1.xy) + distance(sb1.xy, sb2.xy);
  } else {
    length_screen = distance(sb0.xy, sb1.xy) + distance(sb1.xy, sb2.xy) +
                    distance(sb2.xy, sb3.xy);
  }

  int segments = 10 * int(length_screen * screenResolution);
  return clamp(segments, 8, 256);
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

vec4 bezier2(vec4 b0, vec4 b1, vec4 b2, float t) {
  float t1 = 1.0 - t;

  vec4 a = t1 * b0 + t * b1;
  vec4 b = t1 * b1 + t * b2;

  return t1 * a + t * b;
}

void createPolyLine(in vec4 b0, in vec4 b1, in vec4 b2, in vec4 b3, in mat4 pv,
                    in int len) {
  for (int i = 0; i < gs_in[0].len; i++) {
    gl_Position = pv * gs_in[0].pts[i];
    EmitVertex();
  }
}

void main() {
  int len = gs_in[0].len;
  vec4 b0 = gs_in[0].pts[0];
  vec4 b1 = gs_in[0].pts[1];
  vec4 b2 = gs_in[0].pts[2];
  vec4 b3 = gs_in[0].pts[3];

  mat4 pv = projection * view;

  if (renderPolyLine) {
    for (int i = 0; i < gs_in[0].len; i++) {
      gl_Position = pv * gs_in[0].pts[i];
      EmitVertex();
    }
    EndPrimitive();
  }

  if (len == 1) {
    gl_Position = pv * b0;
    EmitVertex();
  } else if (len == 2) {
    gl_Position = pv * b0;
    EmitVertex();
    gl_Position = pv * b1;
    EmitVertex();
  } else if (len == 3) {
    int segmentsCount = calculateLength(b0, b1, b2, b3, pv, len);
    for (int i = 0; i <= segmentsCount; ++i) {
      float t = float(i) / segmentsCount;
      vec4 point = bezier2(b0, b1, b2, t);
      gl_Position = pv * point;
      EmitVertex();
    }
  } else if (len == 4) {
    int segmentsCount = calculateLength(b0, b1, b2, b3, pv, len);
    for (int i = 0; i <= segmentsCount; ++i) {
      float t = float(i) / segmentsCount;
      vec4 point = bezier3(b0, b1, b2, b3, t);
      gl_Position = pv * point;
      EmitVertex();
    }
  }

  EndPrimitive();
}
