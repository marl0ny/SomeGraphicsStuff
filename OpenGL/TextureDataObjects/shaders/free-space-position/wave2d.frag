#VERSION_NUMBER_PLACEHOLDER
/* Green function propagator for the wave equation in 2D
 obtained from a table in:

 - Wikipedia contributors. (2022, August 5).
 [Green's function](https://en.wikipedia.org/wiki/Green%27s_function).
 In Wikipedia, The Free Encyclopedia.

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

const float PI = 3.141592653589793;

uniform float c;
uniform float t;
uniform vec4 amplitudes;
uniform vec2 r0;
uniform vec2 dimensions;


void main() {
    vec2 r = vec2(
        UV[0]*dimensions[0],
        UV[1]*dimensions[1]);
    float u = 1.0/(2.0*PI*c*sqrt(c*c*t*t - dot(r - r0, r - r0)));
    fragColor = u*amplitudes;
}


