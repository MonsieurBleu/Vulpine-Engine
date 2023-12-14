#ifndef FNCT_NOISE_GLSL
#define FNCT_NOISE_GLSL

// https://www.shadertoy.com/view/wtsSW4
float gold_noise3(in vec3 coordinate, in float seed){
    return 0.5 - fract(tan(distance(coordinate*(seed+PHI*00000.1), vec3(PHI*00000.1, PI*00000.1, E)))*SQR2*10000.0);
}

vec2 goldNoiseCustom(in vec3 coordinate, in float seed)
{
    return vec2(0.5) - vec2(2.0)*vec2(
        fract(tan(distance(coordinate.xy*PHI, coordinate.xy)*seed)*coordinate.x),
        fract(tan(distance(coordinate.zx*PHI, coordinate.zx)*seed)*coordinate.z)
        );
}

// https://github.com/tt6746690/computer-graphics-shader-pipeline/blob/master/src/random2.glsl
vec2 random2(vec3 st){
  vec2 S = vec2( dot(st,vec3(127.1,311.7,783.089)),
             dot(st,vec3(269.5,183.3,173.542)) );
  return fract(sin(S)*43758.5453123);
}

#endif
