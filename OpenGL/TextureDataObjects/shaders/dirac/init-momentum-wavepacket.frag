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

/* Initialiize a wavepacket in momentum space.
The wavepacket is expressed in terms of the free periodic
solutions of the Dirac equation, which are also used
in the split-step-momentum3d.frag shader.
*/

// Dimensions
uniform vec3 dimensions;
uniform ivec3 texelDimensions;

// Wavepacket expectation values.
uniform vec3 position;
uniform vec3 momentum;

// Standard deviation (momentum space)
uniform vec3 sigma;

uniform float m;
uniform float c;

uniform int spinorIndex;
const int TOP = 0;
const int BOTTOM = 1;

const int DIRAC_REP = 0;
const int WEYL_REP = 1;
uniform int representation;

#define complex vec2
#define complex2 vec4

const int BY_SPINOR = 0;
const int BY_ENERGY = 1;
uniform int initializationMode;
uniform complex uPosECoeff; // up positive
uniform complex dPosECoeff; // down positive
uniform complex uNegECoeff; // up negative
uniform complex dNegECoeff; // down negative

const float PI = 3.141592653589793;

complex mul(complex z1, complex z2) {
    return complex(z1.x*z2.x - z1.y*z2.y, 
                   z1.x*z2.y + z1.y*z2.x);
}

complex conj(complex z) {
    return vec2(z.x, -z.y);
}

complex innerProd(complex2 z1, complex2 z2) {
    return mul(conj(z1.rg), z2.rg) + mul(conj(z1.ba), z2.ba);
}

complex2 c1C2(complex z, complex2 z2) {
    complex a = complex(z2[0], z2[1]);
    complex b = complex(z2[2], z2[3]);
    return complex2(complex(z.x*a.x - z.y*a.y, z.x*a.y + z.y*a.x),
                    complex(z.x*b.x - z.y*b.y, z.x*b.y + z.y*b.x));
}

complex frac(complex z1, complex z2) {
    complex invZ2 = conj(z2)/(z2.x*z2.x + z2.y*z2.y);
    return mul(z1, invZ2);
}

float pow2(float val) {
    return val*val;
}

vec3 to3DTextureCoordinates(vec2 uv) {
    int length3D = texelDimensions[2];
    float u = mod(uv[0]*float(length3D), 1.0);
    float v = uv[1];
    float w = (floor(uv[0]*float(length3D)) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

vec3 getMomentum() {
    float u, v, w;
    float width, height, length_;
    int texelWidth, texelHeight, texelLength;
    width = dimensions[0];
    height = dimensions[1];
    length_ = dimensions[2];
    texelWidth = texelDimensions[0];
    texelHeight = texelDimensions[1];
    texelLength = texelDimensions[2];
    if (texelDimensions.z != 0) {
        vec3 uvw = to3DTextureCoordinates(UV);
        u = uvw[0], v = uvw[1], w = uvw[2];
    } else {
        u = UV[0], v = UV[1];
    }
    float freqU = ((u < 0.5)? u: -1.0 + u)*float(texelWidth) - 0.5;
    float freqV = ((v < 0.5)? v: -1.0 + v)*float(texelHeight) - 0.5;
    float freqW = ((w < 0.5)? w: -1.0 + w)*float(texelLength) - 0.5;
    return vec3(2.0*PI*freqU/width, 2.0*PI*freqV/height, 
                (texelDimensions.z != 0)? 2.0*PI*freqW/length_: 0.0);
}

/*Compute the spin up eigenvector for a Pauli matrix oriented in an
arbitrary direction. Although easily double by pencil and paper,
this was instead done using 
Python with [Sympy](https://www.sympy.org/en/index.html).
The representation used for the Pauli matrices are found here:
https://en.wikipedia.org/wiki/Pauli_matrices.

>>> from sympy import Symbol, sqrt
>>> from sympy import Matrix
>>> nx = Symbol('nx', real=True)
>>> ny = Symbol('ny', real=True)
>>> nz = Symbol('nz', real=True)
>>> n = sqrt(nx**2 + ny**2 + nz**2)
>>> H = Matrix([[nz, nx - 1j*ny],
>>>             [nx + 1j*ny, -nz]])
>>> eigvects, diag_matrix = H.diagonalize(normalize=True)
>>> eigvects = eigvects.subs(n, 'n')
>>> print(eigvects, diag_matrix)

*/
complex2 getSpinUpState(vec3 orientation, float len) {
    float n = len;
    float nx = orientation.x, ny = orientation.y, nz = orientation.z;
    complex az = complex(1.0, 0.0);
    complex bz = complex(0.0, 0.0);
    complex a = frac(complex(n + nz, 0.0),
                     complex(nx, ny)*sqrt((nz + n)*(nz + n)/(nx*nx + ny*ny)
                                          + 1.0));
    complex b = complex(1.0/sqrt((nz + n)*(nz + n)/(nx*nx + ny*ny) + 1.0),
                        0.0);
    if ((nx*nx + ny*ny) == 0.0)
        return complex2(az, bz);
    return complex2(a, b);
}

/*Compute the spin down eigenvector for a Pauli matrix oriented in an
arbitrary direction. See documentation for getSpinUpState for more 
information.*/
complex2 getSpinDownState(vec3 orientation, float len) {
    float n = len;
    float nx = orientation.x, ny = orientation.y, nz = orientation.z;
    complex az = complex(0.0, 0.0);
    complex bz = complex(1.0, 0.0);
    complex a = frac(complex(-n + nz, 0.0),
                     complex(nx, ny)*sqrt((nz - n)*(nz - n)/(nx*nx + ny*ny)
                                          + 1.0));
    complex b = complex(1.0/sqrt((nz - n)*(nz - n)/(nx*nx + ny*ny) + 1.0),
                        0.0);
    if ((nx*nx + ny*ny) == 0.0)
        return complex2(az, bz);
    return complex2(a, b);
}

/*
Find the eigenvalues of a real symmetric 2x2 matrix.
The argument i indexes which eigenvalue to get, 
while d0 and d1 denote the top and bottom diagonal elements respectively.
The variable nd corresponds to the non-diagonal element.

It is assumed that the eigenvalues of the matrix is purely real,
which implies that 
    d0*d0 - 2*d0*d1 + d1*d1 + 4*nd*nd > 0.

The eigenvalues and eigenvectors are found using Python
with [Sympy](https://www.sympy.org/en/index.html):

>>> from sympy import Matrix
>>> from sympy import Symbol
>>> d0 = Symbol('d0', real=True)
>>> d1 = Symbol('d1', real=True)
>>> nd = Symbol('nd', real=True)
>>> mat = Matrix([[d0, nd], [nd, d1]])
>>> mat_eigenvects = mat.eigenvects()
>>> for eig_info in mat_eigenvects:
>>>     eigval, degeneracy, eigvects = eig_info
>>>     print('Eigenvalue: ', eigval, '\nDegeneracy: ', degeneracy)
>>>     for eigvect in eigvects:
>>>         eigvect_normalized = eigvect/eigvect.norm()
>>>         eigvect_normalized.simplify()
>>>         print(eigvect_normalized)
>>>         print()

*/
float eigenvalueRealSymmetric2x2(int i, float d0, float d1, float nd) {
    if (nd == 0.0)
        return (i == 0)? d0: d1;
    if (i == 0)
        return d0/2.0 + d1/2.0
                 - sqrt(d0*d0 - 2.0*d0*d1 + d1*d1 + 4.0*nd*nd)/2.0;
    else
        return d0/2.0 + d1/2.0 
                 + sqrt(d0*d0 - 2.0*d0*d1 + d1*d1 + 4.0*nd*nd)/2.0;
}

/*
Find the eigenvectors of a real symmetric 2x2 matrix.
The argument i indexes which eigenvector to get, 
while d0 and d1 denote the top and bottom diagonal elements respectively.
The variable nd corresponds to the non-diagonal element.

It is assumed that the eigenvalues of the matrix is purely real,
which implies that 
    d0*d0 - 2*d0*d1 + d1*d1 + 4*nd*nd > 0.

The eigenvalues and eigenvectors are found using Python
with [Sympy](https://www.sympy.org/en/index.html):

>>> from sympy import Matrix
>>> from sympy import Symbol
>>> d0 = Symbol('d0', real=True)
>>> d1 = Symbol('d1', real=True)
>>> nd = Symbol('nd', real=True)
>>> mat = Matrix([[d0, nd], [nd, d1]])
>>> mat_eigenvects = mat.eigenvects()
>>> for eig_info in mat_eigenvects:
>>>     eigval, degeneracy, eigvects = eig_info
>>>     print('Eigenvalue: ', eigval, '\nDegeneracy: ', degeneracy)
>>>     for eigvect in eigvects:
>>>         eigvect_normalized = eigvect/eigvect.norm()
>>>         eigvect_normalized.simplify()
>>>         print(eigvect_normalized)
>>>         print()

*/
vec2 eigenvectorRealSymmetric2x2(int i, float d0, float d1, float nd) {
    if (nd == 0.0)
        return (i == 0)? vec2(1.0, 0.0): vec2(0.0, 1.0);
    if (i == 0)
        return vec2(
            (d0 - d1 - sqrt(d0*d0 - 2.0*d0*d1 + d1*d1 + 4.0*nd*nd))
             / (nd*sqrt(pow2((-d0 + d1
                              + sqrt(d0*d0 - 2.0*d0*d1 + d1*d1 + 4.0*nd*nd)
                             )/nd
                            ) + 4.0
                        )
                ),
            2.0/sqrt(pow2((-d0 + d1
                           + sqrt(d0*d0 - 2.0*d0*d1 + d1*d1 + 4.0*nd*nd)
                          )/nd
                          ) + 4.0)
        );
    else
        return vec2(
            (d0 - d1 + sqrt(d0*d0 - 2.0*d0*d1 + d1*d1 + 4.0*nd*nd))
             / (nd*sqrt(pow2((d0 - d1 
                              + sqrt(d0*d0 - 2.0*d0*d1 + d1*d1 + 4.0*nd*nd)
                             )/nd
                            ) + 4.0
                        )
                ),
            2.0/sqrt(pow2((d0 - d1
                           + sqrt(d0*d0 - 2.0*d0*d1 + d1*d1 + 4.0*nd*nd)
                          )/nd
                          ) + 4.0)
        );
}


void main() {
    
    vec3 pVec = getMomentum();

    float px = pVec.x, py = pVec.y, pz = pVec.z;
    float p2 = px*px + py*py + pz*pz;
    float p = sqrt(p2);
    float mc = m*c;

    complex2 up = getSpinUpState(pVec, p);
    complex2 down = getSpinDownState(pVec, p);

    // - + - +
    float e0, e1, e2, e3;
    vec2 vUp0, vUp1, vDown0, vDown1;

    if (representation == DIRAC_REP) {
        e0 = eigenvalueRealSymmetric2x2(0, mc, -mc, p);
        vUp0 = eigenvectorRealSymmetric2x2(0, mc, -mc, p);
        e1 = eigenvalueRealSymmetric2x2(1, mc, -mc, p);
        vUp1 = eigenvectorRealSymmetric2x2(1, mc, -mc, p);
        e2 = eigenvalueRealSymmetric2x2(0, mc, -mc, -p);
        vDown0 = eigenvectorRealSymmetric2x2(0, mc, -mc, -p);
        e3 = eigenvalueRealSymmetric2x2(1, mc, -mc, -p);
        vDown1 = eigenvectorRealSymmetric2x2(1, mc, -mc, -p);
    } else if (representation == WEYL_REP) {
        e0 = eigenvalueRealSymmetric2x2(0, -p, p, mc);
        vUp0 = eigenvectorRealSymmetric2x2(0, -p, p, mc);
        e1 = eigenvalueRealSymmetric2x2(1, -p, p, mc);
        vUp1 = eigenvectorRealSymmetric2x2(1, -p, p, mc);
        e2 = eigenvalueRealSymmetric2x2(0, p, -p, mc);
        vDown0 = eigenvectorRealSymmetric2x2(0, p, -p, mc);
        e3 = eigenvalueRealSymmetric2x2(1, p, -p, mc);
        vDown1 = eigenvectorRealSymmetric2x2(1, p, -p, mc);
    }

    // -
    complex2 v00 = vUp0[0]*up;
    complex2 v01 = vUp0[1]*up; 
    // +
    complex2 v10 = vUp1[0]*up;
    complex2 v11 = vUp1[1]*up;
    // -
    complex2 v20 = vDown0[0]*down;
    complex2 v21 = vDown0[1]*down;
    // +
    complex2 v30 = vDown1[0]*down;
    complex2 v31 = vDown1[1]*down;

    complex2 s0, s1;
    if (initializationMode == BY_ENERGY) {
        s0 = c1C2(uNegECoeff, v00) + c1C2(uPosECoeff, v10)
             + c1C2(dNegECoeff, v20) + c1C2(dPosECoeff, v30);
        s1 = c1C2(uNegECoeff, v01) + c1C2(uPosECoeff, v11)
             + c1C2(dNegECoeff, v21) + c1C2(dPosECoeff, v31);
    } else {
        s0 = complex2(1.0, 0.0, 0.0, 0.0);
        s1 = complex2(0.0, 0.0, 0.0, 0.0);
    }

    vec3 xVec = position;
    vec3 p0Vec = momentum;
    vec3 s = vec3(sigma.x, sigma.y, sigma.z);
    // For 2D sigma.z will not have an effect on the distribution
    // as long as it is non-zero, which this check ensures.
    // s.z = (texelDimensions.z == 0 && sigma.z == 0.0)? 1.0: sigma.z;
    float dist = exp(-0.5*dot((pVec - p0Vec)/s, (pVec - p0Vec)/s));
    complex e = complex(cos(dot(pVec, xVec)), sin(dot(pVec, xVec)));

    fragColor = dist*c1C2(conj(e), ((spinorIndex == TOP)? s0: s1));

}
