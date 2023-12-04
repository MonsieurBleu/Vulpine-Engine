#version 460

#include uniform/Base3D.glsl
#include uniform/Model3D.glsl

layout (location = 0) out vec4 fragColor;

in vec2 uv;
in vec4 color;
in flat int type;
in float aspectRatio;

const float SMOOTHSTEP_BORDER = 0.01;
// const float SMOOTHSTEP_BORDER_SQUARED = SMOOTHSTEP_BORDER*SMOOTHSTEP_BORDER;

const float borderSize = 0.075;

vec2 arCorrection;

float drawCircle(vec2 inUv)
{
    float l = length(inUv);
    fragColor.a *= smoothstep(1.0, 1.0 - SMOOTHSTEP_BORDER, l);
    return l;
}

float drawSquareRounded(float cornerSize, vec2 inUv)
{
    inUv = abs(inUv*arCorrection);
    vec2 c = arCorrection - cornerSize; 

    vec2 cUv = max(inUv, c)-c;
    float inBorder = 1.0 - step(cUv.x, 0.f)*step(cUv.y, 0.f);
    cUv *= inBorder/cornerSize;

    float cBorderSize = cornerSize-borderSize;
    float b = smoothstep(cBorderSize, cBorderSize+SMOOTHSTEP_BORDER, drawCircle(cUv)*cornerSize);
    vec2 bUv = arCorrection-inUv;
    float b2 = smoothstep(borderSize+SMOOTHSTEP_BORDER, borderSize, min(bUv.x, bUv.y));

    b = mix(b, b2, 1.0-inBorder);

    return mix(b, b2, 1.0-inBorder);;
}

float drawSquare(vec2 inUv)
{
    inUv = abs(inUv*arCorrection);
    vec2 bUv = arCorrection-inUv;
    float b = smoothstep(borderSize+SMOOTHSTEP_BORDER, borderSize, min(bUv.x, bUv.y));
    return b;
}

void main()
{
    fragColor = color;
    float border = 0.f;
    vec2 uvAR = uv;
    arCorrection = aspectRatio > 1.f ? vec2(aspectRatio, 1.0) : vec2(1.0, 1.0/aspectRatio);

    switch(type)
    {
        case 0 : border = drawSquare(uv); break;
        case 1 : border = drawSquareRounded(0.5, uv); break;
        case 2 : border = drawCircle(uv); break;
    }

    fragColor.rgb *= (1.0-border)*0.5 + 0.5;
}
