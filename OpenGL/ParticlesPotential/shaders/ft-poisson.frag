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

#define complex2 vec4

const float PI = 3.141592653589793;

uniform int texelWidth;
uniform int texelHeight;
uniform sampler2D densityTex;
uniform complex2 constant;


complex2 multiply(complex2 w, complex2 z) {
    return complex2(w[0]*z[0] - w[1]*z[1], w[0]*z[1] + w[1]*z[0],
                    w[2]*z[2] - w[3]*z[3], w[2]*z[3] + w[3]*z[2]);
}


void main() {
    float w = float(texelWidth);
    float h = float(texelHeight);
    float kx = 2.0*PI*(((UV[0] < 0.5)? UV[0]: -1.0 + UV[0]) - 0.5/w);
    float ky = 2.0*PI*(((UV[1] < 0.5)? UV[1]: -1.0 + UV[1]) - 0.5/h);
    if (kx == 0.0 && ky == 0.0) fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    else fragColor = constant*texture2D(densityTex, UV)/(kx*kx + ky*ky);
}