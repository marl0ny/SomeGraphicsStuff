#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec2 UV;
out vec4 fragColor;
#else
#define fragColor gl_FragColor
varying highp vec2 UV;
#endif

uniform int wStack; // Number of xy texture blocks along vertical
uniform int hStack; // Similar to wStack but along the horizontal

uniform vec3 r0;
uniform vec3 colour;
uniform vec3 sigma;


vec3 to3DTextureCoordinates(vec2 uv) {
    float texelLength = float(wStack*hStack);
    float wIndex = floor(uv[1]*float(hStack))*float(wStack)
                    + floor(uv[0]*float(wStack));
    return vec3(
        mod(uv[0]*float(wStack), 1.0),
        mod(uv[1]*float(hStack), 1.0),
        (wIndex + 0.5)/texelLength
    );
}


void main() {
    vec3 uvw = to3DTextureCoordinates(UV);
    // fragColor = vec4(colour*uvw, 1.0);
    float gaussian = exp(-0.5*pow((uvw[0] - r0[0])/sigma[0], 2.0)
                         -0.5*pow((uvw[1] - r0[1])/sigma[1], 2.0)
                         -0.5*pow((uvw[2] - r0[2])/sigma[2], 2.0));
    fragColor = vec4(gaussian*colour, 1.0);

}