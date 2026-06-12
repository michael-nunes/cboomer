#version 130
in vec2  aPos;
in vec2  aTexCoord;
out vec2 texcoord;

uniform vec2  cameraPos;
uniform vec2  windowSize;
uniform vec2  screenshotSize;
uniform float cameraZoom;

vec2 to_world(vec2 pos) {
	vec2 ratio = windowSize / screenshotSize / cameraZoom;
	return  (pos / screenshotSize * 2.0 - 1.0) / ratio;
}

void main() {
	gl_Position = vec4(to_world(aPos.xy-cameraPos*vec2(1.0, -1.0)),0.0,1.0);
	texcoord = aTexCoord;
}
