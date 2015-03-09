#version 120

varying vec4 texcoord;
varying vec3 normal;

void main() {
	gl_Position = ftransform();
	
	texcoord = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	
	normal = gl_Normal;
}