#version 150

out vec4 outputColor;
uniform float time;
uniform sampler2DRect brush;
uniform sampler2DRect backbuffer;

uniform vec2 resolution;

in vec2 texCoordVarying;
in vec2 uv;

float sinN(float t) { return (sin(t)+1)/2;}
float rand(const in float n){return fract(sin(n) * 1e4);}
// quantize and input number [0, 1] to quantLevels levels
float quant(float num, float quantLevels){
    float roundPart = floor(fract(num*quantLevels)*2.);
    return (floor(num*quantLevels))/quantLevels;
}
vec2 quant(vec2 num, float quantLevels){
    return vec2(quant(num.x, quantLevels), quant(num.y, quantLevels));
}

void main()
{
    vec4 col = texture(brush, uv*resolution);
    vec4 bb = texture(backbuffer, quant(uv+vec2(sin(time+uv.y*6)*0.003, -0.005),400+sinN(time)*100)*resolution);
    outputColor = col+bb;
    if(uv.y < 0.001) outputColor = vec4(0, 0, 0, 0);
}
