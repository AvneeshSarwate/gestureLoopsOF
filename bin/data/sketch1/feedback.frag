#version 150

out vec4 outputColor;
uniform float time;
uniform sampler2DRect brush;
uniform sampler2DRect backbuffer;
uniform sampler2DRect delayedFrame;
uniform vec2 resolution;
uniform int schemeInd;

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
vec2 rotate(vec2 space, vec2 center, float amount){
    return vec2(cos(amount) * (space.x - center.x) + sin(amount) * (space.y - center.y) + center.x,
        cos(amount) * (space.y - center.y) - sin(amount) * (space.x - center.x) + center.y);
}

void scheme1() {
    vec4 col = texture(brush, uv*resolution);
    vec4 bb = texture(backbuffer, quant(uv+vec2(sin(time+uv.y*6)*0.003, -0.005),400+sinN(time)*100)*resolution);
    outputColor = col+bb;
    if(uv.y < 0.001) outputColor = vec4(0, 0, 0, 0);
}

void scheme2() {
    vec4 col = texture(brush, uv*resolution);
    vec4 delay = texture(delayedFrame, rotate(uv, vec2(0.5), 0.02) *resolution);
    vec4 bbN = texture(backbuffer, uv*resolution);
//    vec4 bb = texture(backbuffer, quant(uv+vec2(sin(time+uv.y*6)*0.003, -0.005),400+sinN(time)*100)*resolution);
    outputColor = col+delay*(1.-pow(sinN(time),2));
    if(uv.y < 0.001) outputColor = vec4(0, 0, 0, 0);
}

void main()
{
    if(schemeInd == 0) outputColor = vec4(1, 0, 0, 1);;
    if(schemeInd == 1) scheme1();
    if(schemeInd == 2) scheme2();
}
