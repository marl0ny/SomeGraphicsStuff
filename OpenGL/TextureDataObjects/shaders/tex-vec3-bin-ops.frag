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

uniform vec3 val;
uniform bool texOnLeft;
uniform sampler2D tex;
uniform int opType;

#define DIV 2
#define MUL 3
#define ADD 4
#define SUB 5
#define MIN 101
#define MAX 102


void main() {
    vec4 texVal = texture2D(tex, UV);
    if (opType == ADD) {
        fragColor = texVal + vec4(val, 0.0);
    } else if (opType == MUL) {
        fragColor = vec4(val*texVal.xyz, texVal.w);
    } else if (opType == SUB) {
        fragColor = (texOnLeft)?
            (texVal - vec4(val, 0.0)): (vec4(val, 0.0) - texVal);
    } else if (opType == DIV) {
        if (texOnLeft) {
            fragColor = vec4(texVal.xyz/val, texVal.w);
        } else {
            fragColor = vec4(val/texVal.xyz, texVal.w);
        }
    }
}

