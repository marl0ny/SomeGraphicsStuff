/* GLSL implementation of pseudorandom values.

Reference:
 Vivo P., Lowe J. Random. In the Book of Shaders.
 https://thebookofshaders.com/10/
 */
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
uniform float scale;
uniform vec3 offset;
uniform bool useSeedVal;
uniform vec2 seedVal1;
uniform vec2 seedVal2;
uniform vec2 seedVal3;

#define PI 3.141592653589793


vec3 rand() {
    if (useSeedVal) {
        return scale*vec3(
            fract(sin(dot(UV, seedVal1))),
            fract(cos(dot(UV, seedVal2))),
            fract(sin(dot(UV, seedVal3)))) + offset; 
    }
    return scale*vec3(
        fract(sin(dot(UV, vec2(2.71828, 137.036)))),
        fract(cos(dot(UV, vec2(82817.2, 630.731)))),
        fract(sin(dot(UV, vec2(123.456, 654.321))))) + offset;
}

void main() {
    // fragColor = vec4(rand(), 1.0);
    fragColor = vec4(vec3(rand()[0]), 1.0);
}
