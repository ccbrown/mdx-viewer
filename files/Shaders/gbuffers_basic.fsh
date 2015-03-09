#version 120

varying vec4 color;
varying vec3 normal;

void main() {

	gl_FragData[0] = color;
	gl_FragData[1] = vec4(vec3(gl_FragCoord.z), 1.0);
	gl_FragData[2] = vec4(normal * 0.5 + 0.5, 1.0);
	
}