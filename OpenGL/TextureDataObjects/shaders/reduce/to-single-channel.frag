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

uniform sampler2D tex;
uniform int size;

void main() {
    vec4 val = texture2D(tex, UV);
    float res = 0.0;
    // No for loop since older versions of OpenGL
    // do not support for loops.
    if (size >= 1)
        res += val[0];
    if (size >= 2)
        res += val[1];
    if (size >= 3)
        res += val[2];
    if (size >= 4)
        res += val[3];
    fragColor = vec4(res, res, res, res);
}
