#version 120

uniform int shadingMode;

uniform sampler2D texture1;
uniform sampler2D texture2;

varying vec4 texcoord1;
varying vec4 texcoord2;

varying vec3 normal;

void main() {

	vec4 s1 = texture2D(texture1, texcoord1.st);
	vec4 s2 = texture2D(texture2, texcoord2.st);

	if (shadingMode == 0) {
		gl_FragData[0] = vec4(s1.rgb, 1.0);
	} else {
		// im not quite sure how the different modes should actually work
		// it would probably look best if they were rendered to a different buffer so i could give them a glowing effect
		// 3 - sunset glow effect
		// 5 - pretty colors
		// 6 - emmissive trim
		// 9 - emmissive lava / rock
		gl_FragData[0] = vec4(max(s1.rgb, s2.rgb * s2.a), 1.0);
	}
	
	gl_FragData[1] = vec4(vec3(gl_FragCoord.z), 1.0);
	gl_FragData[2] = vec4(normal * 0.5 + 0.5, 1.0);
	
}