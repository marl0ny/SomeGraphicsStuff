#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec2 UV;
out vec4 fragColor;
#define texture2D texture
#else
#define fragColor gl_FragColor
varying highp vec2 UV;
#endif

#define complex vec2

#define PI 3.141592653589793

uniform float amplitude;
uniform float sigma;
uniform vec2 r0;
uniform ivec2 wavenumber;
uniform int type;

const int GAUSSIAN_WAVEPACKET = 0;
const int INV_SQUARE = 1;
const int SWIRL = 2;

complex conj(complex z) {
    return complex(z.r, -z.g);
}

void main() {
    float y = UV[1] - 0.5;
    float x = UV[0] - 0.5;
    // vec2 aField = 10.0*vec2(-y, x);
    vec2 aField = 0.0*vec2(-y, x);
    float u = UV[0] - r0[0];
    float v = UV[1] - r0[1];
    float nx = float(wavenumber[0]);
    float ny = float(wavenumber[1]);
    complex phase = complex(cos(2.0*PI*(nx*u + ny*v)),
                            sin(2.0*PI*(nx*u + ny*v)));
    if (type == GAUSSIAN_WAVEPACKET) {
        float amp = amplitude*exp(-0.5*(u*u + v*v)/(sigma*sigma));
        complex psi = amp*phase;
        fragColor = vec4(aField, psi);
    } else if (type == INV_SQUARE) {
        float mag 
            = 1.0 - ((abs(u) < sigma)? ((abs(v) < sigma)? 1.0: 0.0): 0.0);
        complex psi = amplitude*mag*phase;
        psi += amplitude*complex(1.0, 0.0)*((abs(u) < sigma)? ((abs(v) < sigma)? 1.0: 0.0): 0.0);
        fragColor = vec4(aField, psi);
    } else if (type == SWIRL) {
        float angle = nx*atan(v/u);
        complex phase = complex(cos(angle), sin(angle));
        complex psi = amplitude*phase;
        fragColor = vec4(aField, psi);
    }
}