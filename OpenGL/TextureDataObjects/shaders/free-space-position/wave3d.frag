#VERSION_NUMBER_PLACEHOLDER
/* 12.55 of these notes gives the Green's function explicitly
for the wave equation: 
https://www.physics.uoguelph.ca/chapter-12-greens-function

References:

 - Wikipedia contributors. (2022, August 5).
 [Green's function](https://en.wikipedia.org/wiki/Green%27s_function).
 In Wikipedia, The Free Encyclopedia.

 - Chapter 12: Green's Function.
 In PHYS*3130 Mathematical Physics Lecture Notes.
 https://www.physics.uoguelph.ca/chapter-12-greens-function

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
uniform vec4 amplitude;
uniform vec4 x0;
uniform float t;
uniform vec4 sigma;
uniform ivec2 texDimensions2D;
uniform ivec3 texDimensions3D;
// uniform ivec4 texDimensions4D;
uniform vec3 dimensions;

float sq(float a) {
    return a*a;
}

void main() {
    vec3 r = vec3(
        UV[0]*dimensions[0],
        UV[1]*dimensions[1],
        0.0
    );
    vec3 r0 = x0.xyz;
    float t0 = x0[3];
    float s = t - t0 - length(r - r0)/c;
    float lengthR2 = c*c*(t - t0)*(t - t0);

    vec4 g = vec4(
        amplitude[0]*exp(-0.5*s*s/sq(sigma[0])),
        amplitude[1]*exp(-0.5*s*s/sq(sigma[1])),
        amplitude[2]*exp(-0.5*s*s/sq(sigma[2])),
        amplitude[3]*exp(-0.5*s*s/sq(sigma[3]))
    )/(4.0*PI*length(r - r0));
}