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

uniform int whichComponent;
uniform int texelWidth;
uniform int texelHeight;
uniform sampler2D tex;


complex2 multiply(complex2 w, complex2 z) {
    return complex2(w[0]*z[0] - w[1]*z[1], w[0]*z[1] + w[1]*z[0],
                    w[2]*z[2] - w[3]*z[3], w[2]*z[3] + w[3]*z[2]);
}


void main() {
    float w = float(texelWidth);
    float h = float(texelHeight);
    float kx = 2.0*PI*(((UV[0] < 0.5)? UV[0]: -1.0 + UV[0]) - 0.5/w);
    float ky = 2.0*PI*(((UV[1] < 0.5)? UV[1]: -1.0 + UV[1]) - 0.5/h);
    complex2 f = texture2D(tex, UV);
    complex2 IMAG_UNIT = complex2(0.0, 1.0, 0.0, 1.0);
    complex2 kxF = multiply(IMAG_UNIT*kx, f);
    complex2 kyF = multiply(IMAG_UNIT*ky, f);
    if (whichComponent == 1)
        fragColor = complex2(kxF[2], kxF[3], kyF[2], kyF[3]);
    else
        fragColor = complex2(kxF[0], kxF[1], kyF[0], kyF[1]);
}