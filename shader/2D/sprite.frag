#version 460

layout (location = 0) out vec4 fragColor;
layout (binding = 0) uniform sampler2D bAtlas;

in vec2 atlasUV;
in vec3 position;

float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec3 bodyColor = vec3(1.f);
    vec3 outlineColor = vec3(0.f);

    vec4 texel = texture(bAtlas, atlasUV);

    // if(c.r <= 0.00001) discard;

    float dist = median(texel.r, texel.g, texel.b);

    float pxRange = 10.0;
  	float pxDist = pxRange * (dist - 0.2);
	// float opacity = clamp(pxDist + 0.5, 0.0, 1.0);
    float opacity = pxDist;
    fragColor = vec4(1.0, 1.0, 1.0, opacity);


    // float BpxRange = 10.0;
  	// float BpxDist = BpxRange * (dist - 0.5);
    // float Bopacity = smoothstep(0.0000001, 0.001, dist);
    // fragColor.rgb = mix(outlineColor, bodyColor, opacity);
    // fragColor.a = Bopacity;
}