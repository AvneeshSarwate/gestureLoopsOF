#version 150

uniform mat4 modelViewProjectionMatrix;

in vec2 texcoord;
in vec4 position;
out vec2 texCoordVarying;
out vec2 uv;

uniform vec2 resolution;
uniform vec4 bbox; //bbox is (x, y, width, height) with x/y being top-left


void main(){
    texCoordVarying = texcoord;
	uv.x = (position.x - bbox.x) / bbox.z;
	uv.y = (position.y - bbox.y) / bbox.w;
	gl_Position = modelViewProjectionMatrix * position;
}
