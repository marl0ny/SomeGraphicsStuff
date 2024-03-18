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


/*
The names of variables follow the convention used on page
68 - 69 of the Computational Electrodynamics book by
Taflove A. and Hagness S. The Finite difference discretization of 
Maxwell's equation is based on eq. 3.7 and 3.8 of the same book (pg. 69),
and a basic introduction to the Yee algorithm is found on pg 78 to 79.
*/

uniform float dt; // timestep
uniform float mu; // magnetic permeability
uniform float sigma; // magnetic loss
uniform sampler2D texCurlEField;
uniform sampler2D texHField; // H field at previous step
uniform sampler2D texM; // Magnetic source


void main() {
    vec3 curlE = texture2D(texCurlEField, UV).xyz;
    vec3 M = texture2D(texM, UV).xyz;
    vec3 H = texture2D(texHField, UV).xyz;
    fragColor = vec4((-(dt/mu)*curlE
                     - (dt/mu)*M + (1.0 - 0.5*(sigma*dt/mu))*H)/
                     (1.0 + 0.5*(sigma*dt/mu)), 1.0);
}

