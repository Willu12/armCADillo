#version 430

layout(quads, equal_spacing, ccw) in;

uniform mat4 view;
uniform mat4 projection;
uniform uint v_patches;
uniform uint u_patches;

out vec2 trim_coord;

const int DEGREE = 3;

// Get control point (4x4 grid assumed)
vec3 p(uint i, uint j) { return gl_in[i * 4 + j].gl_Position.xyz; }

// Uniform cubic B-spline basis — find knot span index
int find_knot_span(float t, int num_ctrl_pts, int degree) {
  // For uniform cubic B-spline with open knots, the span is floor(t * (n - d))
  int n = num_ctrl_pts - 1;
  int m = n + degree + 1;
  float span = t * float(n - degree + 1);
  return clamp(int(floor(span)), degree, n);
}

// De Boor for 1D cubic B-spline
vec3 de_boor_1d(float t, int num_ctrl_pts, vec3 ctrl_pts[4]) {
  vec3 d0 = ctrl_pts[0];
  vec3 d1 = ctrl_pts[1];
  vec3 d2 = ctrl_pts[2];
  vec3 d3 = ctrl_pts[3];

  float u = t;

  // First round
  vec3 d10 = mix(d0, d1, u);
  vec3 d11 = mix(d1, d2, u);
  vec3 d12 = mix(d2, d3, u);

  // Second round
  vec3 d20 = mix(d10, d11, u);
  vec3 d21 = mix(d11, d12, u);

  // Final point
  return mix(d20, d21, u);
}

// Evaluate B-spline surface
vec3 bspline_surface(float u, float v) {
  vec3 temp[4];

  // De Boor in v direction (rows)
  for (int i = 0; i < 4; ++i) {
    vec3 col[4];
    for (int j = 0; j < 4; ++j) {
      col[j] = p(i, j);
    }
    temp[i] = de_boor_1d(v, 4, col);
  }

  // Then de Boor in u direction (columns)
  return de_boor_1d(u, 4, temp);
}

void main() {
  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;

  float u_glob = (gl_PrimitiveID / v_patches + u) / u_patches;
  float v_glob = (gl_PrimitiveID % v_patches + v) / v_patches;

  trim_coord = vec2(u_glob, v_glob);

  vec4 position = vec4(bspline_surface(u, v), 1.0f);
  gl_Position = projection * view * position;
}
