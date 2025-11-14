#version 430

layout(isolines, equal_spacing, ccw) in;

uniform mat4 view;
uniform mat4 projection;
uniform uint direction;
uniform bool polyline;
uniform float offset_surface;
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

vec3 bezier3_derivative(vec3 b0, vec3 b1, vec3 b2, vec3 b3, float t) {
  float t1 = 1.0f - t;
  vec3 tangent = 3.0 * t1 * t1 * (b1 - b0) + 6.0 * t1 * t * (b2 - b1) +
                 3.0 * t * t * (b3 - b2);
  return tangent;
}

vec3 bicubic_bezier(float u, float v) {
  vec3 p0 = bezier3(p(0, 0), p(0, 1), p(0, 2), p(0, 3), v);
  vec3 p1 = bezier3(p(1, 0), p(1, 1), p(1, 2), p(1, 3), v);
  vec3 p2 = bezier3(p(2, 0), p(2, 1), p(2, 2), p(2, 3), v);
  vec3 p3 = bezier3(p(3, 0), p(3, 1), p(3, 2), p(3, 3), v);
  return bezier3(p0, p1, p2, p3, u);
}

void bicubic_bezier_derivatives(float u, float v, out vec3 du, out vec3 dv) {
  vec3 p0 = bezier3(p(0, 0), p(0, 1), p(0, 2), p(0, 3), v);
  vec3 p1 = bezier3(p(1, 0), p(1, 1), p(1, 2), p(1, 3), v);
  vec3 p2 = bezier3(p(2, 0), p(2, 1), p(2, 2), p(2, 3), v);
  vec3 p3 = bezier3(p(3, 0), p(3, 1), p(3, 2), p(3, 3), v);

  vec3 dp0 = bezier3_derivative(p(0, 0), p(0, 1), p(0, 2), p(0, 3), v);
  vec3 dp1 = bezier3_derivative(p(1, 0), p(1, 1), p(1, 2), p(1, 3), v);
  vec3 dp2 = bezier3_derivative(p(2, 0), p(2, 1), p(2, 2), p(2, 3), v);
  vec3 dp3 = bezier3_derivative(p(3, 0), p(3, 1), p(3, 2), p(3, 3), v);

  du = bezier3_derivative(p0, p1, p2, p3, u);
  dv = bezier3(dp0, dp1, dp2, dp3, u);
}

vec3 normal(float u, float v) {
  vec3 du, dv;
  bicubic_bezier_derivatives(u, v, du, dv);
  vec3 norm = normalize(cross(du, dv));
  return norm;
}

void main() {
  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;

  float uSub = gl_TessLevelOuter[0] - 1;
  v = (uSub + 1) * v / uSub;

  if (direction == 1) {
    float temp = u;
    u = v;
    v = temp;
  }

  uint col = gl_PrimitiveID / v_patches;
  uint row = gl_PrimitiveID % v_patches;

  float u_glob = (col + v) / u_patches;
  float v_glob = (row + u) / v_patches;

  trim_coord = vec2(v_glob, u_glob);

  vec3 pos = bicubic_bezier(u, v);

  if (offset_surface != 0.f) {
    pos += 0.4 * offset_surface * normal(u, v);
  }
  gl_Position = projection * view * vec4(pos, 1.0);
}
