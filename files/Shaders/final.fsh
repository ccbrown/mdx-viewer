#version 120

uniform sampler2D gcolor;
uniform sampler2D gdepth;
uniform sampler2D gnormal;

varying vec4 texcoord;

void main() {
	gl_FragColor = vec4(texture2D(gcolor, texcoord.st).rgb, 1.0);
}
