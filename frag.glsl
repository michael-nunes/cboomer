#version 130
in vec2 texcoord;
uniform vec2 cameraPos;
uniform float cameraZoom;
uniform sampler2D tex;
out mediump vec4 finalColor;


float rand(vec2 co) {
	return fract(sin(dot(co.xy,vec2(12.9898,78.233)))) * 43758.5453;
}	

void main() {
	finalColor = texture(tex, texcoord);
}
