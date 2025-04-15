#version 430

layout(location = 0) in int len;
layout(location = 1) in vec3 pts0;
layout(location = 2) in vec3 pts1;
layout(location = 3) in vec3 pts2;
layout(location = 4) in vec3 pts3;

out VS_OUT {
  int len;
  vec4 pts[4];
}
vs_out;

void main() {
  vs_out.len = len;
  vs_out.pts[0] = vec4(pts0, 1.0);
  vs_out.pts[1] = vec4(pts1, 1.0);
  vs_out.pts[2] = vec4(pts2, 1.0);
  vs_out.pts[3] = vec4(pts3, 1.0);
}