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

uniform sampler2D posVelTex;

// Textures for storing the external forces.
// The nomenclature convention for these textures are:
//  - Either f0Tex or f1Tex,
//    Where f0Tex is evaluated at t + 0*dt
//    and   f1Tex              at t + 1*dt.
//  - f[n]D[m]Tex where [n]D[m] denotes a fraction.
/     For example, f12D13Tex is evaluated at t + 12*dt/13.
uniform sampler2D f0Tex;
uniform sampler2D f1D4Tex;
uniform sampler2D f3D8Tex;
uniform sampler2D f1D2Tex;
uniform sampler2D f12D13Tex;
uniform sampler2D f1Tex;

uniform float errorTolerance;
uniform float dt;

const int RETURN_ADAPTIVE_STEP = 0;
const int RETURN_RESULT = 1;
uniform int returnType;

const int DO_RK4 = 0;
const int DO_RK45 = 1;
uniform int integrationType;


vec4 f(vec4 posVel, sampler2D forceTex) {
    vec2 position = posVel.xy;
    vec2 velocity = posVel.zw;
    return vec4(velocity, 
                texture2D(forceTex, position));
}

/*References:

https://fncbook.github.io/fnc/ivp/rk.html

https://math.okstate.edu/people/yqwang/teaching/
    math4513_fall11/Notes/rungekutta.pdf

*/
vec4 integrationRK4(vec4 posVel) {
    vec4 k1 = dt*f(posVel, f0Tex);
    vec4 k2 = dt*f(posVel + k1/2.0, f1D2Tex);
    vec4 k3 = dt*f(posVel + k2/2.0, f1D2Tex);
    vec4 k4 = dt*f(posVel + k3, f1Tex);
    return posVel + k1/6.0 + k2/3.0 + k3/3.0 + k4/6.0;
}

vec4 integrationRK45(vec4 posVel, int returnType) {
    vec4 k1 = dt*f(posVel, f0Tex);
    vec4 k2 = dt*f(posVel + (1.0/4.0)*k1, f1D4Tex);
    vec4 k3 = dt*f(posVel + (3.0/32.0)*k1
                          + (9.0/32.0)*k2, f3D8Tex);
    vec4 k4 = dt*f(posVel + (1932.0/2197.0)*k1
                          - (7200.0/2197.0)*k2
                          + (7296.0/2197.0)*k3, f12D13Tex);
    vec4 k5 = dt*f(posVel + (439.0/216.0)*k1 
                          - (8.0)*k2
                          + (3680.0/513.0)*k3 
                          - (845.0/4104.0)*k4, f1Tex);
    vec4 k6 = dt*f(posVel - (8.0/27.0)*k1
                          + (2.0)*k2 
                          - (3544.0/2565.0)*k3 
                          + (1859.0/4104.0)*k4 
                          - (11.0/40.0)*k5, f1D2Tex);
    vec4 sol1 = posVel + (25.0/216.0)*k1 
                       + (1408.0/2565.0)*k3
                       + (2197.0/4104.0)*k4
                       - (1.0/5.0)*k5;
    vec4 sol2 = posVel + (16.0/135.0)*k1
                       + (6656.0/12825.0)*k2
                       + (28561.0/56430.0)*k3
                       - (9/50.0)*k4
                       + (2.0/55.0)*k5;
    float r = (1.0/dt)*length(sol2 - sol1);
    float delta = 0.8*pow(errorTolerance/r, 1.0/4.0);
    if (returnType == RETURN_RESULT) {
        return sol1;
    } else {
        return vec4(delta*dt);
    }
}

void main() {
    vec4 posVel = texture2D(posVelTex, UV);
    if (integrationType == DO_RK4) {
        fragColor = integrationRK4(posVel);
    } else if (integrationType == DO_RK45) {
        fragColor = integrationRK45(posVel, returnType);
    }
}