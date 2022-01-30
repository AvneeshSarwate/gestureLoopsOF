#version 150

out vec4 outputColor;
uniform float time;
uniform sampler2DRect brush;
uniform sampler2DRect backbuffer;
uniform sampler2DRect delayedFrame;
uniform sampler2DRect video;
uniform vec2 resolution;
uniform int schemeInd;
uniform vec2 pts[10];

in vec2 texCoordVarying;
in vec2 uv;

float PI = 3.14159;

float sinN(float t) { return (sin(t)+1)/2;}
float cosN(float t) { return (cos(t)+1)/2;}
float rand(const in float n){return fract(sin(n) * 1e4);}
float rand2(float f) {vec2 n = vec2(f, -f); return (fract(1e4 * sin(17.0 * n.x + n.y * 0.1) * (0.1 + abs(sin(n.y * 13.0 + n.x))))-0.5)*0.02;}
// quantize and input number [0, 1] to quantLevels levels
float quant(float num, float quantLevels){
    float roundPart = floor(fract(num*quantLevels)*2.);
    return (floor(num*quantLevels))/quantLevels;
}
vec2 quant(vec2 num, float quantLevels){
    return vec2(quant(num.x, quantLevels), quant(num.y, quantLevels));
}
float quant2(float num, float quantLevels){
    float roundPart = floor(fract(num*quantLevels)*2.);
    return (floor(num*quantLevels)+roundPart)/quantLevels;
}
vec2 quant2(vec2 num, float quantLevels){
    return vec2(quant2(num.x, quantLevels), quant2(num.y, quantLevels));
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
    vec4 c1 = texture(tex, rotate(nn+vec2(0, dist), nn, PI/4)      ).rgba;
    vec4 c2 = texture(tex, rotate(nn+vec2(0, -dist), nn, PI/4)     ).rgba;
    vec4 c3 = texture(tex, rotate(nn+vec2(dist, 0), nn, PI/4)      ).rgba;
    vec4 c4 = texture(tex, rotate(nn+vec2(-dist, 0), nn, PI/4)     ).rgba;
    vec4 c5 = texture(tex, rotate(nn+vec2(dist, dist), nn, PI/4)   ).rgba;
    vec4 c6 = texture(tex, rotate(nn+vec2(-dist, -dist), nn, PI/4) ).rgba;
    vec4 c7 = texture(tex, rotate(nn+vec2(dist, -dist), nn, PI/4)  ).rgba;
    vec4 c8 = texture(tex, rotate(nn+vec2(-dist, dist), nn, PI/4)  ).rgba;
    
    return (c1+c2+c3+c4+c5+c6+c7+c8)/8.;
}

vec3 coordWarp(vec2 stN, float t2){
    vec2 warp = stN;
    
    float rad = .5;
    
    for (float i = 0.0; i < 20.; i++) {
        vec2 p = vec2(sinN(t2* rand(i+1.) * 1.3 + i), cosN(t2 * rand(i+1.) * 1.1 + i));
        warp = length(p - stN) <= rad ? mix(warp, p, 1. - length(stN - p)/rad)  : warp;
    }
    
    return vec3(warp, distance(warp, stN));
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
    float r = distance(uv, pts[0]);
    float g = distance(uv, pts[1]);
    float b = distance(uv, pts[2]);
    vec4 brush = texture(brush, uv*resolution);
    vec4 bb = texture(backbuffer, uv*resolution);
    bb = avgSample(backbuffer, mix(uv, cent, 0.001)*resolution, 1);
//    r = distance(uv, pts[0]) < 0.1 ? 1 : r;
//    g = distance(uv, pts[1]) < 0.1 ? 1 : g;
//    b = distance(uv, pts[2]) < 0.1 ? 1 : b;
    
    vec4 col = vec4(r, g, b, 1);
    float lastFeedback = bb.a;
    float fdbk = 1;
    float decay = 0.01;
    if(brush.r == 1) {
        col = vec4(r, g, b, fdbk);
    } else {
        fdbk = max(0, fdbk - 0.01);
        col = bb;
        col.a = lastFeedback-decay;
    }
//    col = col*col;
    outputColor = col;
}

void scheme4() {
    vec2 cent = vec2(0.5);
    vec2 uvN = mix(quant2(uv, 50), quant(uv, 50), pow(sinN(time*0.5), 5));
    vec4 bb = texture(backbuffer, uvN*resolution);
    bb = avgSample(backbuffer, mix(uvN, cent, 0.001)*resolution, 1);

    vec4 col;
    
    vec2 warpN = coordWarp(uvN, 423).xy;
    warpN = mix(uvN, warpN, sinN(time));
//    warpN = quant(warpN, 50);
    bool in1 = distance(warpN, pts[0]) < 0.1;
    bool in2 = distance(warpN, pts[1]) < 0.1;
    bool in3 = distance(warpN, pts[2]) < 0.1;
    bool draw = in1 || in2 || in3;
    
    vec3 gloop;
    if(in1) gloop = vec3(0.3, 0.5, .11);
    if(in2) gloop = vec3(0.35, 0.44, .61)*0;
    if(in3) gloop = vec3(0.45, 0.04, .41);
    
    float lastFeedback = bb.a;
    float fdbk = 1;
    float decay = 0.01;
    if(draw) {
        col = vec4(gloop.rgb, fdbk);
    } else {
        fdbk = max(0, fdbk - 0.01);
        col = bb;
        col.a = lastFeedback-decay;
    }
//    col = col*col;
    outputColor = col;
}

void scheme5() {
    vec2 cent = vec2(0.5);
    float circTime = time*0.3;
    vec2 circ = vec2(sinN(circTime), cosN(circTime));
    
    vec2 p0 = pts[0];
    vec2 p1 = mix(pts[1], circ, 0.5);
    vec2 p2 = pts[2];
    
    vec2 uvN = uv; //mix(quant2(uv, 5000), quant(uv, 50), pow(sinN(time*0.5), 5)*0.);
    float yTime = (sinN(time+uv.y*PI*5)+1)*0.5;
    vec2 bbN = mix(rotate(uvN, cent, (distance(p0, p2)-0.65)*0.1), cent, 0.0);
    bbN = mix(bbN, cent, sin(time)*0.003);
    vec4 bb = texture(backbuffer, bbN*resolution);
    bb = avgSample(backbuffer, bbN*resolution, 1);

    vec4 col;
    
    vec2 warpN = coordWarp(uvN, 423).xy;
    warpN = mix(uvN, warpN, yTime);
//    warpN = quant(warpN, 50);
    bool in1 = distance(warpN, p0) < 0.1;
    bool in2 = distance(warpN, p1) < 0.1;
    bool in3 = distance(warpN, p2) < 0.1;
    bool draw = in1 || in2 || in3;
    
    vec3 gloop;
    if(in1) gloop = vec3(0.2, 0.4, .61) + vec3(sin(time*1.1), sin(time*1.2), sin(time*1.3))*0.015;
    if(in2) gloop = vec3(pow(sinN(time*0.2), 0.4));
    if(in3) gloop = vec3(0.85, 0.54, .31) + vec3(sin(time*1.1), sin(time*1.2), sin(time*1.3))*0.015;
    
    float lastFeedback = bb.a;
    float fdbk = 1;
    float decay = 0.007;
    if(draw) {
        col = vec4(gloop.rgb, fdbk);
    } else {
        fdbk = max(0, fdbk - 0.01);
        col = bb-decay*0.3;
        col.a = lastFeedback-decay;
    }
//    col = col*col;
    outputColor = col;
}



void main()
{
    if(schemeInd == 0) outputColor = vec4(1, 0, 0, 1);;
    if(schemeInd == 1) scheme1();
    if(schemeInd == 2) scheme2();
    if(schemeInd == 3) scheme3();
    if(schemeInd == 4) scheme4();
    if(schemeInd == 5) scheme5();
}
