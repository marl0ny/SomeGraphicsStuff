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

uniform float epsilon;
uniform float sigma;

/* uniform bool useMaterials;
uniform float epsilonConstant; // electrical permittivity
uniform float sigmaConstant; // electric conductivity
uniform sampler2D texEpsilon; // electrical permittivity
uniform sampler2D texSigma; // electric conductivity*/

uniform sampler2D texCurlHField;
uniform sampler2D texEField; // E field at previous step
uniform sampler2D texJ; // Current


void main() {
    /* float epsilon 
        = (useMaterials)? texture2D(texEpsilon, UV).x: epsilonConstant;
    float sigma
        = (useMaterials)? texture2D(texSigma, UV).x: sigmaConstant;*/
    vec3 curlH = texture2D(texCurlHField, UV).xyz;
    vec3 J = texture2D(texJ, UV).xyz;
    vec3 E = texture2D(texEField, UV).xyz;
    float eps = epsilon;
    fragColor = vec4(((dt/eps)*curlH 
                      - (dt/eps)*J + (1.0 - 0.5*(sigma*dt/eps))*E)/
                     (1.0 + 0.5*(sigma*dt/eps)), 1.0);
}