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

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform int c0;
uniform int c1;
uniform int c2;
uniform int c3;
uniform int d0;
uniform int d1;
uniform int d2;
uniform int d3;

void main() {
    vec4 val1 = texture2D(tex1, UV);
    vec4 val2 = texture2D(tex2, UV);
    float x1 = (c0 < 4 && c0 >= 0)? val1[c0]: 0.0;
    float y1 = (c1 < 4 && c1 >= 0)? val1[c1]: 0.0;
    float z1 = (c2 < 4 && c2 >= 0)? val1[c2]: 0.0;
    float w1 = (c3 < 4 && c3 >= 0)? val1[c3]: 0.0;
    float x2 = (d0 < 4 && d0 >= 0)? val2[d0]: 0.0;
    float y2 = (d1 < 4 && d1 >= 0)? val2[d1]: 0.0;
    float z2 = (d2 < 4 && d2 >= 0)? val2[d2]: 0.0;
    float w2 = (d3 < 4 && d3 >= 0)? val2[d3]: 0.0;
    fragColor = vec4(x1, y1, z1, w1) + vec4(x2, y2, z2, w2);
}