#version 430

layout(vertices = 16) out;

uniform uint u_subdivisions;
uniform uint v_subdivisions;
uniform uint direction;

void main() {
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  if (direction == 0) {
    gl_TessLevelOuter[0] = u_subdivisions + 1;
    gl_TessLevelOuter[1] = v_subdivisions;
  } else {
    gl_TessLevelOuter[0] = v_subdivisions + 1;
    gl_TessLevelOuter[1] = u_subdivisions;
  }
}
