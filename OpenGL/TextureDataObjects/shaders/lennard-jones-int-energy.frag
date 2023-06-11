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

/*
auto potential_func = [&](Texture2DData &r) -> Texture2DData {
        double sigma12 = pow(sigma, 12);
        double sigma6 = pow(sigma, 6);
        return (4.0*epsilon)*(sigma12*pow(r, -12) - sigma6*pow(r, -6));
    };
    */

void main() {
    vec2 r0 = texture2D(positionsTex, vec2(UV[0], 0.5)).xy;
    vec2 r1 = texture2D(positionsTex, vec2(UV[1], 0.5)).xy;
    vec2 r = r1 - r0;
    float sigma = 1.0;
    float epsilon = 1.0;
    float sigma12 = pow(sigma, 12);
    float sigma6 = pow(sigma, 6);
    float eps = 1e-2;
    float s1 = (4.0*epsilon)*(-12.0*sigma12*pow(length(r) + eps, -14));
    float s2 = (4.0*epsilon)*(6*sigma6*pow(length(r) + eps, -8));
    vec2 res = r*(s1 + s2);
    fragColor = vec4(res, res);
}
