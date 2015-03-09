#version 120

uniform sampler2D texture;

varying vec4 texcoord;
varying vec4 color;
varying vec3 normal;

void main() {

	gl_FragData[0] = vec4(texture2D(texture, texcoord.st).rgb, 1.0);
	gl_FragData[1] = vec4(vec3(gl_FragCoord.z), 1.0);
	gl_FragData[2] = vec4(normal * 0.5 + 0.5, 1.0);
	
}