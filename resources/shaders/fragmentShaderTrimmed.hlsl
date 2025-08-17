#version 430

layout(binding = 0) uniform sampler2D trimmer;
uniform vec4 color;
uniform int trim;
in vec2 trim_coord;

out vec4 frag_color;

void main() {

  frag_color = vec4(trim_coord.x, trim_coord.y, 0.f, 1.f);
  vec4 texSample = texture(trimmer, trim_coord);
  if (trim == 1 && texSample.a < 0.5) {
    discard;
  } else {
    frag_color = color;
  }
}
