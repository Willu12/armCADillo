#version 430

layout(isolines, equal_spacing, ccw) in;

uniform mat4 view;
uniform mat4 projection;
uniform uint direction;
uniform bool renderPolyLine;
uniform uint u_patches;
uniform uint v_patches;
out vec2 trim_coord;

vec3 p(uint up, uint vp) { return gl_in[up * 4 + vp].gl_Position.xyz; }

vec3 bezier3(vec3 b0, vec3 b1, vec3 b2, vec3 b3, float t) {
  float t1 = 1.0f - t;
  b0 = mix(b0, b1, t);
  b1 = mix(b1, b2, t);
  b2 = mix(b2, b3, t);
  b0 = mix(b0, b1, t);
  b1 = mix(b1, b2, t);
  return mix(b0, b1, t);
}

vec3 bicubic_bezier(float u, float v) {
  vec3 p0 = bezier3(p(0, 0), p(0, 1), p(0, 2), p(0, 3), v);
  vec3 p1 = bezier3(p(1, 0), p(1, 1), p(1, 2), p(1, 3), v);
  vec3 p2 = bezier3(p(2, 0), p(2, 1), p(2, 2), p(2, 3), v);
  vec3 p3 = bezier3(p(3, 0), p(3, 1), p(3, 2), p(3, 3), v);
  return bezier3(p0, p1, p2, p3, u);
}

void main() {
  float u = gl_TessCoord.x;
  float v = float(gl_TessCoord.y) * float(gl_TessLevelOuter[0]) /
            float(gl_TessLevelOuter[0] - 1);

  if (direction == 0) {
    float temp = u;
    u = v;
    v = temp;
  }

  float u_glob = (gl_PrimitiveID / v_patches + u) / u_patches;
  float v_glob = (gl_PrimitiveID % v_patches + v) / v_patches;

  trim_coord = vec2(v_glob, u_glob);

  vec3 pos = bicubic_bezier(u, v);
  gl_Position = projection * view * vec4(pos, 1.0);
}
