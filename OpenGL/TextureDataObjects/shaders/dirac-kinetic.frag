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

/* The Dirac equation using an arbitrary four-vector potential and
with constants like c and hbar still explicitly shown is written
on pg 566 (eq. 20.2.2) of Principles of Quantum Mechanics by Shankar.

 The momentum space propagator for the Dirac equation using the 
 split operator method in the Dirac representation is derived in 
 II.3 of this article by Bauke and Keitel: https://arxiv.org/abs/1012.3911.
 To derive the momentum space propagator for the Weyl representation,
 the gamma matrices as given on (3.25) in pg. 41 of 
 An Introduction to Quantum Field Theory by Michael Peskin and Daniel Schoeder
 are used.
*/
uniform int numberOfDimensions;
uniform ivec2 texelDimensions2D;
uniform vec2 dimensions2D;
uniform ivec3 texelDimensions3D;
uniform vec3 dimensions3D;

uniform sampler2D uTex;
uniform sampler2D vTex;
uniform float dt;
uniform float m;
uniform float c;
uniform float hbar;

uniform int spinorIndex;
const int TOP = 0;
const int BOTTOM = 1;

const int DIRAC_REP = 0;
const int WEYL_REP = 1;
const int ORIGINAL_DIRAC_IMPLEMENTATION = 2;
uniform int representation;

#define complex vec2
#define complex2 vec4

const float PI = 3.141592653589793;


vec3 to3DTextureCoordinates(vec2 uv) {
    int length3D = texelDimensions3D[2];
    float u = mod(uv[0]*float(length3D), 1.0);
    float v = uv[1];
    float w = (floor(uv[0]*float(length3D)) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

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

vec3 getMomentum() {
    float u, v, w;
    float width, height, length_;
    int texelWidth, texelHeight, texelLength;
    if (numberOfDimensions == 3) {
        width = dimensions3D[0];
        height = dimensions3D[1];
        length_ = dimensions3D[2];
        texelWidth = texelDimensions3D[0];
        texelHeight = texelDimensions3D[1];
        texelLength = texelDimensions3D[2];
        vec3 uvw = to3DTextureCoordinates(UV);
        u = uvw[0], v = uvw[1], w = uvw[2];
    } else {
        width = dimensions2D[0];
        height = dimensions2D[1];
        length_ = 0.0;
        texelWidth = texelDimensions2D[0];
        texelHeight = texelDimensions2D[1];
        texelLength = 0;
        u = UV[0], v = UV[1], w = 0.0;
    }
    float freqU = ((u < 0.5)? u: -1.0 + u)*float(texelWidth) - 0.5;
    float freqV = ((v < 0.5)? v: -1.0 + v)*float(texelHeight) - 0.5;
    float freqW = ((w < 0.5)? w: -1.0 + w)*float(texelLength) - 0.5;
    if (representation == ORIGINAL_DIRAC_IMPLEMENTATION) {
        if (freqU == 0.0) freqU += 5e-8;
        if (freqV == 0.0) freqV += 5e-8;
        if (freqW == 0.0) freqW += 5e-8;
    }
    return vec3(2.0*PI*freqU/width, 2.0*PI*freqV/height, 
                2.0*PI*freqW/length_);
}

/* 
Compute the eigenvectors for a Pauli matrix oriented in an
arbitrary dimension. Although easily double by pencil and paper,
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
Compute the eigenvectors for a Pauli matrix oriented in an
arbitrary dimension. Although easily double by pencil and paper,
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

float pow2(float val) {
    return val*val;
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

    // Get the eigenvectors of the Pauli matrix that is
    // orientated in the same direction as the momentum
    complex2 up = getSpinUpState(pVec, p);
    complex2 down = getSpinDownState(pVec, p);

    // Scaled eigenvalues of the kinetic energy matrix for the given momenta.
    float e0, e1, e2, e3;

    vec2 vUp0, vUp1, vDown0, vDown1;
    // These will be used to compute the actual corresponding eigenvectors
    // of the eigenvalues declared previously.

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


    } else if (representation == ORIGINAL_DIRAC_IMPLEMENTATION) {

        float omega = sqrt(mc*mc + p2);
        float den1 = p*sqrt((mc - omega)*(mc - omega) + p2);
        float den2 = p*sqrt((mc + omega)*(mc + omega) + p2);

        // The matrix U for the momentum step, where U e^{E} U^{\dagger}.
        // This is found by diagonalizing the matrix involving the mass
        // and momentum terms using a computer algebra system like Sympy,
        // which can be expressed as  U E inv(U), where E is the diagonal
        // matrix of eigenvalues and U is the matrix of eigenvectors. 
        // This is following what is similarly done in II.3 of this
        // article by Bauke and Keitel:
        // https://arxiv.org/abs/1012.3911

        float   matUDag00 = pz*(mc - omega)/den1;
        complex matUDag01 = complex(px, -py)*(mc - omega)/den1;
        float   matUDag02 = p2/den1;
        float   matUDag03 = 0.0;
        complex matUDag10 = complex(px, py)*(mc - omega)/den1;
        float   matUDag11 = -pz*(mc - omega)/den1;
        float   matUDag12 = 0.0;
        float   matUDag13 = p2/den1;
        float   matUDag20 = pz*(mc + omega)/den2;
        complex matUDag21 = complex(px, -py)*(mc + omega)/den2;
        float   matUDag22 = p2/den2;
        float   matUDag23 = 0.0;
        complex matUDag30 = complex(px, py)*(mc + omega)/den2;
        float   matUDag31 = -pz*(mc + omega)/den2;
        float   matUDag32 = 0.0;
        float   matUDag33 = p2/den2;

        float   matU00 = matUDag00;
        complex matU01 = conj(matUDag10);
        float   matU02 = matUDag20;
        complex matU03 = conj(matUDag30);
        complex matU10 = conj(matUDag01);
        float   matU11 = matUDag11;
        complex matU12 = conj(matUDag21);
        float   matU13 = matUDag31;
        float   matU20 = matUDag02;
        float   matU21 = matUDag12;
        float   matU22 = matUDag22;
        float   matU23 = matUDag32;
        float   matU30 = matUDag03;
        float   matU31 = matUDag13;
        float   matU32 = matUDag23;
        float   matU33 = matUDag33;
        
        complex2 u = texture2D(uTex, UV);
        complex2 v = texture2D(vTex, UV);
        complex psi0 = u.xy;
        complex psi1 = u.zw;
        complex psi2 = v.xy;
        complex psi3 = v.zw;

        float cos_val = cos(omega*c*dt/hbar);
        float sin_val = sin(omega*c*dt/hbar); 
        complex e1 = complex(cos_val, sin_val); 
        complex e2 = complex(cos_val, -sin_val);

        complex phi0 = matUDag00*psi0 + mul(matUDag01, psi1)
                        + matUDag02*psi2 + matUDag03*psi3;
        complex phi1 = mul(matUDag10, psi0) + matUDag11*psi1
                        + matUDag12*psi2 + matUDag13*psi3;
        complex phi2 = matUDag20*psi0 + mul(matUDag21, psi1)
                        + matUDag22*psi2 + matUDag23*psi3;
        complex phi3 = mul(matUDag30, psi0) + matUDag31*psi1
                        + matUDag32*psi2 + matUDag33*psi3;
        
        complex e1Phi0 = mul(e1, phi0);
        complex e1Phi1 = mul(e1, phi1);
        complex e2Phi2 = mul(e2, phi2);
        complex e2Phi3 = mul(e2, phi3);

        psi0 = matU00*e1Phi0 + mul(matU01, e1Phi1)
                + matU02*e2Phi2 + mul(matU03, e2Phi3);
        psi1 = mul(matU10, e1Phi0) + matU11*e1Phi1
                + mul(matU12, e2Phi2) + matU13*e2Phi3;
        psi2 = matU20*e1Phi0 + matU21*e1Phi1
                + matU22*e2Phi2 + matU23*e2Phi3;
        psi3 = matU30*e1Phi0 + matU31*e1Phi1
                + matU32*e2Phi2 + matU33*e2Phi3;
        
        complex2 psi01 = complex2(psi0, psi1);
        complex2 psi23 = complex2(psi2, psi3);

        fragColor = (spinorIndex == TOP)? psi01: psi23;
        return;

    }

    // Compute the eigenvectors of the kinetic energy matrix for
    // the given momenta
    complex2 v00 = vUp0[0]*up;
    complex2 v01 = vUp0[1]*up; 

    complex2 v10 = vUp1[0]*up;
    complex2 v11 = vUp1[1]*up;

    complex2 v20 = vDown0[0]*down;
    complex2 v21 = vDown0[1]*down;

    complex2 v30 = vDown1[0]*down;
    complex2 v31 = vDown1[1]*down;

    // Get each spinor component of the wave function
    complex2 s0 = texture2D(uTex, UV);
    complex2 s1 = texture2D(vTex, UV);

    // Get the diagonal form of the wave function with respect to
    // the Kinetic energy matrix at the given momenta
    complex d0 = innerProd(v00, s0) + innerProd(v01, s1);
    complex d1 = innerProd(v10, s0) + innerProd(v11, s1);
    complex d2 = innerProd(v20, s0) + innerProd(v21, s1);
    complex d3 = innerProd(v30, s0) + innerProd(v31, s1);

    // Advance the wave function in time
    d0 = mul(complex(cos(e0*c*dt/hbar), -sin(e0*c*dt/hbar)), d0);
    d1 = mul(complex(cos(e1*c*dt/hbar), -sin(e1*c*dt/hbar)), d1);
    d2 = mul(complex(cos(e2*c*dt/hbar), -sin(e2*c*dt/hbar)), d2);
    d3 = mul(complex(cos(e3*c*dt/hbar), -sin(e3*c*dt/hbar)), d3); 

    // Transform the wave function back to its initial representation
    s0 = c1C2(d0, v00) + c1C2(d1, v10) + c1C2(d2, v20) + c1C2(d3, v30);
    s1 = c1C2(d0, v01) + c1C2(d1, v11) + c1C2(d2, v21) + c1C2(d3, v31);

    fragColor = (spinorIndex == TOP)? s0: s1;

}

