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
    vec4 texel = texture(bAtlas, atlasUV);

    // if(c.r <= 0.00001) discard;

    float dist = median(texel.r, texel.g, texel.b);

    float pxRange = 1.0;
  	float pxDist = pxRange * (dist - 0.5);
	float opacity = clamp(pxDist + 0.5, 0.0, 1.0);
    fragColor = vec4(1.0, 1.0, 1.0, opacity);


    // float BpxRange = 1.0;
  	// float BpxDist = BpxRange * (dist - 0.5);
    // float Bopacity = smoothstep(0.0000001, 0.1, dist);
    // fragColor.rgb = mix(vec3(1), vec3(0), opacity);
    // fragColor.a = Bopacity;
}