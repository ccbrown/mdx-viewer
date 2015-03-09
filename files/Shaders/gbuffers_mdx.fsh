#version 120

uniform int shadingMode;

uniform float alpha;

uniform vec4 color;

#define SHADING_MODE_OPAQUE 0

uniform sampler2D texture;

varying vec4 texcoord;
varying vec3 normal;

void main() {

	if (shadingMode == SHADING_MODE_OPAQUE) {
		gl_FragData[0] = vec4(texture2D(texture, texcoord.st).rgb, 1.0);
	} else {
		vec4 sample = texture2D(texture, texcoord.st);
		gl_FragData[0] = color * vec4(sample.rgb, alpha * sample.a);
	}
	
	gl_FragData[1] = vec4(vec3(gl_FragCoord.z), 1.0);
	gl_FragData[2] = vec4(normal * 0.5 + 0.5, 1.0);
	
}