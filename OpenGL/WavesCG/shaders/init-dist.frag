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

uniform vec4 amplitude;
uniform float sigma_x;
uniform float sigma_y;
uniform float u0;
uniform float v0;



const float TAU = 6.283185307179586;



void main() {
    float u = UV[0] - u0;
    float v = UV[1] - v0;
    float sx = sigma_x;
    float sy = sigma_y;
    fragColor = amplitude*exp(-u*u/(2.0*sx*sx))*exp(-v*v/(2.0*sy*sy));
}