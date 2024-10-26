#VERSION_NUMBER_PLACEHOLDER
/* The equation of motion for a Gaussian wave packet in free space is derived
in pg 151-154 of Shankar, with the final result in eq. (5.1.15). For this
shader the equation of motion was obtained with Sympy by integrating an
initial Gaussian wave packet with the free-space position space propagator 
(5.1.10) (integral expression in eq. 5.1.12).


Reference:
Shankar, R. (1994). Simple Problems in One Dimension.
In Principles of Quantum Mechanics, chapter 5. Springer.
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

uniform float hbar;
uniform float m;
uniform float t;
uniform float amplitude;
uniform vec3 r0;
uniform vec3 p0;
uniform vec3 sigma;
uniform ivec2 texDimensions2D;
// uniform ivec3 texDimensions3D;
// uniform ivec4 texDimensions4D;
uniform vec3 dimensions;

#define complex vec2

complex REAL_PART = complex(1.0, 0.0);
complex IMAG_UNIT = complex(0.0, 1.0);


float re(complex z) {
    return z.x;
}

float im(complex z) {
    return z.y;
}

complex conj(complex z) {
    return complex(z.x, -z.y);
}

complex cExp(complex z) {
    return complex(exp(z.x)*cos(z.y), exp(z.x)*sin(z.y));
}

complex cInv(complex z) {
    return conj(z)/(z.x*z.x + z.y*z.y);
}

complex cMul(complex z, complex w) {
    return complex(z.x*w.x - z.y*w.y,
                   z.x*w.y + z.y*w.x);
}

complex cSquare(complex z) {
    return cMul(z, z);
}

complex cSqrt(complex z) {
    float absZ = length(z);
    if (absZ == 0.0)
        return complex(0.0, 0.0);
    float angle = atan(z.y, z.x);
    return sqrt(absZ)*complex(cos(angle/2.0), sin(angle/2.0));
}


/*
expr = sqrt(pi)
        *sqrt(1/(1/(2*sigma**2) - I*m/(2*hbar*t)))
        *exp((x0/sigma**2 - I*m*y/(hbar*t) + I*p/hbar)**2
                / (2/sigma**2 - 2*I*m/(hbar*t)))
        *exp(-x0**2/(2*sigma**2) + I*m*y**2/(2*hbar*t))
*/
complex get1DWavefunc(float x0, float p, float sigma, float y) {
    complex f1 = cSqrt(m*cInv(2.0*PI*hbar*t*IMAG_UNIT));
    float f2 = 1.0/(sigma*sqrt(2.0*PI));
    complex a = cSqrt(cInv(REAL_PART/(2.0*sigma*sigma) 
                           - IMAG_UNIT*m/(2.0*hbar*t)));
    complex exponent 
        = cMul(cSquare(REAL_PART*x0/(sigma*sigma) 
                       - IMAG_UNIT*m*y/(hbar*t)
                       + IMAG_UNIT*p/hbar),
               cInv(2.0*REAL_PART/(sigma*sigma)
                    - 2.0*IMAG_UNIT*m/(hbar*t)))
        - REAL_PART*x0*x0/(2.0*sigma*sigma)
        + IMAG_UNIT*m*y*y/(2.0*hbar*t);
    return cMul(f1*f2, cMul(a, cExp(exponent)));
    // return cMul(sqrt(PI)*f1*f2, cMul(cMul(a, b), c));
}

void main() {
    vec3 r = vec3(
        UV[0]*dimensions[0],
        UV[1]*dimensions[1],
        0.0 // UV[2]*dimensions[2],
    );
    complex psiX = get1DWavefunc(r0.x, p0.x, sigma.x, r.x);
    complex psiY = get1DWavefunc(r0.y, p0.y, sigma.y, r.y);
    complex psi = amplitude*cMul(psiX, psiY);
    fragColor = vec4(psi, psi);
}


