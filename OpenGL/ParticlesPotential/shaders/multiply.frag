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

uniform sampler2D tex1;
uniform sampler2D tex2;


void main() {
    fragColor = texture2D(tex1, UV)*texture2D(tex2, UV);
}