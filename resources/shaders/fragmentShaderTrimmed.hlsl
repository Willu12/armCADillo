#version 430

layout(binding = 0) uniform sampler2D trimmer;
// uniform vec3 color;
uniform int trim;
in vec2 trim_coord;

out vec4 frag_color;

void main() {

  vec4 texSample = texture(trimmer, trim_coord);
  if (trim == 1 && texSample.a < 0.5) {
    discard;
  } else {
    frag_color = vec4(1.0);
  }
}
