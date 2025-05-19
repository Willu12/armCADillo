#version 430

layout(vertices = 16) out;

uniform uint u_subdivisions;
uniform uint v_subdivisions;
uniform int direction;

void main() {
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  if (gl_InvocationID == 0) {
    uint u_sub = u_subdivisions;
    uint v_sub = v_subdivisions;

    // Swap if direction is 1
    if (direction == 1) {
      uint temp = u_sub;
      u_sub = v_sub;
      v_sub = temp;
    }
    gl_TessLevelOuter[0] = u_sub;
    gl_TessLevelOuter[1] = v_sub;
    gl_TessLevelOuter[2] = u_sub;
    gl_TessLevelOuter[3] = v_sub;

    gl_TessLevelInner[0] = v_sub;
    gl_TessLevelInner[1] = u_sub;
  }
}