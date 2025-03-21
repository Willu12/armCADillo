#version 330
in vec3 worldPos;
layout(location = 0) out vec4 FragColor;

uniform float gridCellSize = 0.125;
uniform float gridMinPixelsBetweenCells = 2.0;
uniform vec4 gridColorThin = vec4(0.5, 0.5, 0.5, 1.0);
uniform vec4 gridColorThick = vec4(0., 0., 0., 1.0);
uniform vec3 cameraWorldPos;
uniform float gridSize = 100.0f;

float log10(float x) { return log(x) / log(10.0); }

void main()
{

    vec2 dvx = vec2(dFdx(worldPos.x), dFdy(worldPos.x));
    vec2 dvy = vec2(dFdx(worldPos.z), dFdy(worldPos.z));

    float lx = length(dvx);
    float ly = length(dvy);

    vec2 dudv = vec2(lx, ly);

    float l = length(dudv);
    float LOD =
        max(0.0, log10(l * gridMinPixelsBetweenCells / gridCellSize) + 1.0);
    float gridCellSizeLod0 = gridCellSize * pow(10.0, floor(LOD));
    float gridCellSizeLod1 = gridCellSizeLod0 * 10.0f;
    float gridCellSizeLod2 = gridCellSizeLod1 * 10.0f;

    dudv *= 4;

    vec2 mod_div_dudv = mod(worldPos.xz, gridCellSizeLod0) / dudv;
    vec2 clamped = clamp(mod_div_dudv, 0.0, 1.0);
    float Lod0a =
        max(1.0 - abs(clamped.x * 2.0 - 1.0), 1.0 - abs(clamped.y * 2.0 - 1.0));

    mod_div_dudv = mod(worldPos.xz, gridCellSizeLod1) / dudv;
    clamped = clamp(mod_div_dudv, 0.0, 1.0);
    float Lod1a =
        max(1.0 - abs(clamped.x * 2.0 - 1.0), 1.0 - abs(clamped.y * 2.0 - 1.0));

    mod_div_dudv = mod(worldPos.xz, gridCellSizeLod2) / dudv;
    clamped = clamp(mod_div_dudv, 0.0, 1.0);
    float Lod2a =
        max(1.0 - abs(clamped.x * 2.0 - 1.0), 1.0 - abs(clamped.y * 2.0 - 1.0));

    float LOD_fade = fract(LOD);

    vec4 Color;
    if (Lod2a > 0.0)
    {
        Color = gridColorThick;
        Color.a *= Lod2a;
    }
    else
    {
        if (Lod1a > 0.0)
        {
            Color = mix(gridColorThick, gridColorThin, LOD_fade);
            Color.a *= Lod1a;
        }
        else
        {
            Color = gridColorThin;
            Color.a *= (Lod0a * (1.0 - LOD_fade));
        }
    }
    float opacityFalloff = (1.0 - clamp(length(worldPos.xz - cameraWorldPos.xz) / (gridSize + 1e-6), 0.0, 1.0));

    Color.a *= opacityFalloff;

    FragColor = Color;
}