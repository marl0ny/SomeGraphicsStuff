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

uniform sampler2D posVelTex;
uniform sampler2D fieldTex;

void main() {
    vec4 posVel = texture2D(posVelTex, UV);
    vec2 pos = posVel.xy/2.0 + vec2(0.5, 0.5);
    fragColor = texture2D(fieldTex, pos);
}