#version 150

out vec4 outputColor;
uniform float time;
uniform sampler2DRect brush;
uniform sampler2DRect backbuffer;
uniform sampler2DRect delayedFrame;
uniform sampler2DRect video;
uniform vec2 resolution;
uniform int schemeInd;

in vec2 texCoordVarying;
in vec2 uv;

float sinN(float t) { return (sin(t)+1)/2;}
float rand(const in float n){return fract(sin(n) * 1e4);}
float rand2(float f) {vec2 n = vec2(f); return (fract(1e4 * sin(17.0 * n.x + n.y * 0.1) * (0.1 + abs(sin(n.y * 13.0 + n.x))))-0.5)*0.002;}
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
//calculate the distance beterrn two colors
// formula found here - https://stackoverflow.com/a/40950076
float colourDistance(vec3 e1, vec3 e2) {
  float rmean = (e1.r + e2.r ) / 2.;
  float r = e1.r - e2.r;
  float g = e1.g - e2.g;
  float b = e1.b - e2.b;
  return sqrt((((512.+rmean)*r*r)/256.) + 4.*g*g + (((767.-rmean)*b*b)/256.));
}

vec4 avgSample(sampler2DRect tex, vec2 nn, float dist){
    vec4 c1 = texture(tex, rotate(nn+vec2(0, dist), nn, rand(1.))      +rand2(1.)).rgba;
    vec4 c2 = texture(tex, rotate(nn+vec2(0, -dist), nn, rand(2.))     +rand2(2.)).rgba;
    vec4 c3 = texture(tex, rotate(nn+vec2(dist, 0), nn, rand(3.))      +rand2(3.)).rgba;
    vec4 c4 = texture(tex, rotate(nn+vec2(-dist, 0), nn, rand(4.))     +rand2(4.)).rgba;
    vec4 c5 = texture(tex, rotate(nn+vec2(dist, dist), nn, rand(5.))   +rand2(5.)).rgba;
    vec4 c6 = texture(tex, rotate(nn+vec2(-dist, -dist), nn, rand(6.)) +rand2(6.)).rgba;
    vec4 c7 = texture(tex, rotate(nn+vec2(dist, -dist), nn, rand(7.))  +rand2(7.)).rgba;
    vec4 c8 = texture(tex, rotate(nn+vec2(-dist, dist), nn, rand(8.))  +rand2(8.)).rgba;
    
    return (c1+c2+c3+c4+c5+c6+c7+c8)/8.;
}

void scheme1() {
    vec4 col = texture(brush, uv*resolution);
    vec4 bb = texture(backbuffer, quant(uv+vec2(sin(time+uv.y*6)*0.003, -0.005),400+sinN(time)*100)*resolution);
    outputColor = col+bb*bb.a;
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

void scheme3() {
    vec2 cent = vec2(0.5);
    vec2 vidN = mod(rotate(uv*3, cent*3, distance(uv, cent)*3*sinN(time)), vec2(1));
    vidN = uv;
    
    vec4 vid = texture(video, vidN*resolution);
    vec4 brush = texture(brush, uv*resolution);
    vec4 bb = texture(backbuffer, uv*resolution);
    vec4 bbAvg = avgSample(backbuffer, uv*resolution, 1);
    vec4 delay0 = texture(delayedFrame, uv *resolution);
    vec4 delay1 = texture(delayedFrame, rotate(uv, vec2(0.5), 0.07) *resolution);
    vec4 delay2 = texture(delayedFrame, (uv+vec2(0, 0.1*sin(quant(time/3., 2)*3.))) *resolution);
    
    vec4 col = vid;
    vec4 delay = delay0;
    
    vec4 warp = (col/delay) - bbAvg*0.1;
    float dist = colourDistance(warp.rgb, bbAvg.rgb);
//    warp = warp + colourDistance(warp.rgb, bbAvg.rgb)*0.05;
    outputColor = mix(warp, bbAvg, .2);
}



void main()
{
    if(schemeInd == 0) outputColor = vec4(0, 0, 0, 1);;
    if(schemeInd == 1) scheme1();
    if(schemeInd == 2) scheme2();
    if(schemeInd == 3) scheme3();
}
