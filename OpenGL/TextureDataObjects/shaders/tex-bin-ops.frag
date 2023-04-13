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

#define complex vec2
#define complex2 vec4

uniform sampler2D texL;
uniform sampler2D texR;
uniform int opType;

#define DIV 2
#define MUL 3
#define ADD 4
#define SUB 5
#define COMPLEX_DIV 12
#define COMPLEX_MUL 13
#define MIN 101
#define MAX 102

complex conj(complex z) {
    return complex(z.x, -z.y);
}

complex inv(complex z) {
    return conj(z)/(z.x*z.x + z.y*z.y);
}

complex mul(complex w, complex z) {
    return complex(w.x*z.x - w.y*z.y, w.x*z.y + w.y*z.x);
}

void main() {
    vec4 valL = texture2D(texL, UV);
    vec4 valR = texture2D(texR, UV);
    if (opType == ADD) {
        fragColor = valL + valR;
    } else if (opType == MUL) {
        fragColor = valL*valR;
    } else if (opType == SUB) {
        fragColor = valL - valR;
    } else if (opType == DIV) {
        fragColor = valL/valR;
    } else if (opType == COMPLEX_MUL) {
        fragColor = complex2(mul(valL.xy, valR.xy),
        		      mul(valL.zw, valR.zw));
    } else if (opType == COMPLEX_DIV) {
        fragColor = complex2(mul(valL.xy, inv(valR.xy)),
        		      mul(valL.zw, inv(valR.zw)));
    } else if (opType == MIN) {
        fragColor = min(valL, valR);
    } else if (opType == MAX) {
        fragColor = max(valL, valR);
    }

}
