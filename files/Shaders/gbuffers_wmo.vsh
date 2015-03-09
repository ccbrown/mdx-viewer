#version 120

varying vec4 texcoord1;
varying vec4 texcoord2;

varying vec3 normal;

void main() {
	gl_Position = ftransform();
	
	texcoord1 = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	texcoord2 = gl_TextureMatrix[1] * gl_MultiTexCoord1;
	
	normal = gl_Normal;
}