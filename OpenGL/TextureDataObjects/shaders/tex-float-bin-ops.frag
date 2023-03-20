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

uniform float val;
uniform bool texOnLeft;
uniform sampler2D tex;
uniform int opType;

#define DIV 2
#define MUL 3
#define ADD 4
#define SUB 5
#define COMPLEX_DIV 12
#define COMPLEX_MUL 13

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
    vec4 texVal = texture2D(tex, UV);
    if (opType == ADD) {
        fragColor = texVal + val;
    } else if (opType == MUL || opType == COMPLEX_MUL) {
        fragColor = val*texVal;
    } else if (opType == SUB) {
        fragColor = (texOnLeft)? (texVal - val): (val - texVal);
    } else if (opType == DIV) {
        fragColor = (texOnLeft)?
            (texVal/val): vec4(val/texVal.x, val/texVal.y,
                               val/texVal.z, val/texVal.w);
    } else if (opType == COMPLEX_DIV) {
        fragColor = (texOnLeft)?
            (texVal/val): complex2(mul(complex(val, 0.0), inv(texVal.xy)),
                                   mul(complex(val, 0.0), inv(texVal.zw)));
    }
}