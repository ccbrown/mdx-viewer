#version 120

varying vec4 color;
varying vec3 normal;

void main() {
	gl_Position = ftransform();
	
	color = gl_Color;
	
	normal = gl_Normal;
}