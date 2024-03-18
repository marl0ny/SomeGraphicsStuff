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

/* (3.73) on pg. 50 of an An Introduction to Quantum Field Theory
 by Michael Peskin and Daniel Schoeder gives the formula for
 computing current. The current is computed with just the
 spinor field and each of the gamma matrices,
 where the gamma matrices in the Weyl representation
 are given on (3.25) in pg. 41 of the same book. For the gamma
 matrices in the Dirac representation, this is obtained from
 the alpha matrices and beta matrix as given in equation 20.1.12 on
 pg 565 of Principles of Quantum Mechanics by Shankar.

*/

/* The type hermitian2x2 corresponds to a two by two
Hermitian matrix. The first two components are the diagonals.
The last two components store the complex off-diagonal element
that's in the first row.*/
#define hermitian2x2 vec4

#define complex vec2
#define complex2 vec4

uniform hermitian2x2 sigmaX;
uniform hermitian2x2 sigmaY;
uniform hermitian2x2 sigmaZ;
uniform sampler2D uTex;
uniform sampler2D vTex;

const int DIRAC_REP = 0;
const int WEYL_REP = 1;
uniform int representation;


float real(complex z) {
    return z[0];
}

float imag(complex z) {
    return z[1];
}

complex conj(complex z) {
    return complex(z[0], -z[1]);
}

complex mul(complex w, complex z) {
    return complex(w[0]*z[0] - w[1]*z[1], w[0]*z[1] + w[1]*z[0]);
}

complex innerProd(complex2 w, complex2 z) {
    return mul(conj(w.rg), z.rg) + mul(conj(w.ba), z.ba);
}

complex2 matrixMul(hermitian2x2 m, complex2 v) {
    complex m00 = complex(m[0], 0.0);
    complex m11 = complex(m[1], 0.0);
    complex m01 = complex(m[2], m[3]);
    complex m10 = conj(m01);
    complex v0 = v.rg;
    complex v1 = v.ba;
    return complex2(mul(m00, v0) + mul(m01, v1),
                    mul(m10, v0) + mul(m11, v1));
}

float expectationValue(hermitian2x2 operator, complex2 state) {
    return real(innerProd(state, matrixMul(operator, state)));
}

void main() {
    complex2 u = texture2D(uTex, UV);
    complex2 v = texture2D(vTex, UV);
    if (representation == DIRAC_REP) {
        fragColor = vec4(2.0*real(innerProd(u, matrixMul(sigmaX, v))),
                         2.0*real(innerProd(u, matrixMul(sigmaY, v))),
                         2.0*real(innerProd(u, matrixMul(sigmaZ, v))),
                         dot(u, u) + dot(v, v)
                         );
    } else {
        fragColor = vec4(
            -expectationValue(sigmaX, u) + expectationValue(sigmaX, v),
            -expectationValue(sigmaY, u) + expectationValue(sigmaY, v),
            -expectationValue(sigmaZ, u) + expectationValue(sigmaZ, v),
            dot(u, u) + dot(v, v)
        );
    }
}
