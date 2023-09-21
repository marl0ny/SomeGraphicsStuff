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

