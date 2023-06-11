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


uniform sampler2D positionsTex;

void main() {
    vec2 r0 = texture2D(positionsTex, vec2(UV[0], 0.5)).xy;
    vec2 r1 = texture2D(positionsTex, vec2(UV[1], 0.5)).xy;
    vec2 r = r1 - r0;
    float e = (length(r) >  0.001)? -1.0/length(r): -1.0/0.001;
    fragColor = (UV[0] == UV[1])? vec4(0.0): vec4(e);
}