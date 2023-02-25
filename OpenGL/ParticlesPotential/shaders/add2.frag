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

uniform vec4 scale1;
uniform vec4 scale2;
uniform sampler2D tex1;
uniform sampler2D tex2;

void main() {
    fragColor = scale1*texture2D(tex1, UV) + scale2*texture2D(tex2, UV);
}