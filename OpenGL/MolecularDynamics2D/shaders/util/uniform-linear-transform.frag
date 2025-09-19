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
// Each row of the 4x4 matrix representation
// of the linear transformation.
uniform vec4 row0, row1, row2, row3;
uniform vec4 b;

void main() {
    vec4 x = texture2D(tex, UV);
    fragColor = vec4(
        dot(row0, x), dot(row1, x), dot(row2, x), dot(row3, x)) + b;
}
