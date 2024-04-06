#VERSION_NUMBER_PLACEHOLDER

#if (__VERSION__ >= 330) || (defined(GL_ES) && __VERSION__ >= 300)
#define texture2D texture
#else
#define texture texture2D
#endif

#if (__VERSION__ > 120) || defined(GL_ES)
precision highp float;
#endif
 
#if __VERSION__ <= 120
varying vec2 UV;
#define fragColor gl_FragColor
#else
in vec2 UV;
out vec4 fragColor;
#endif

#define complex vec2
#define complex2 vec4

uniform vec2 val;
uniform int opType;
uniform bool texOnLeft;
uniform sampler2D tex;

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
    vec4 texVal = texture2D(tex, UV);
    if (opType == ADD) {
        fragColor = texVal + vec4(val, val);
    } else if (opType == MUL) {
        fragColor = vec4(val*texVal.xy, val*texVal.zw);
    } else if (opType == SUB) {
        fragColor = (texOnLeft)?
            (texVal - vec4(val, val)): (vec4(val, val) - texVal);
    } else if (opType == DIV) {
        if (texOnLeft) {
            fragColor = texVal/vec4(val, val);
        } else {
            fragColor = vec4(val, val)/texVal;
        }
    } else if (opType == COMPLEX_MUL) {
        fragColor = complex2(mul(texVal.xy, val), mul(texVal.zw, val));
    } else if (opType == COMPLEX_DIV) {
        if (texOnLeft) {
            fragColor = complex2(mul(texVal.xy, inv(val)),
                                 mul(texVal.zw, inv(val)));
        } else {
            fragColor = complex2(mul(val, inv(texVal.xy)),
                                 mul(val, inv(texVal.zw)));
        }
    }
}
