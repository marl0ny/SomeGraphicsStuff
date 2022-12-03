#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec2 UV;
out vec4 fragColor;
#define texture2D texture
#else
#define fragColor gl_FragColor
varying highp vec2 UV;
#endif

// uniform sampler2D tex;

uniform float t;

void main() {
    fragColor = vec4(cos(UV.x*t)/4.0, sin(UV.y*t)/4.0, 0.0, 1.0);
}