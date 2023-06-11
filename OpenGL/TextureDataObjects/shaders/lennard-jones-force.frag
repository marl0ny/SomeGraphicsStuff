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

uniform float sigma;
uniform float epsilon;
uniform sampler2D positionsTex;

void main() {
    vec2 r0 = texture2D(positionsTex, vec2(UV[0], 0.5)).xy;
    vec2 r1 = texture2D(positionsTex, vec2(UV[1], 0.5)).xy;
    vec2 r = r1 - r0;
    float sigma12 = pow(sigma, 12);
    float sigma6 = pow(sigma, 6);
    float reg = 0.0;
    float s1 = (4.0*epsilon)*(-12.0*sigma12*pow(length(r), -14));
    float s2 = (4.0*epsilon)*(6*sigma6*pow(length(r), -8));
    vec2 res = -r*(s1 + s2);
    fragColor = (UV[0] == UV[1])? vec4(0.0): vec4(res, res);
}
