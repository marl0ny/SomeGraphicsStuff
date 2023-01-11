#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec2 UV;
out vec4 fragColor;
#else
#define fragColor gl_FragColor
varying highp vec2 UV;
#endif

uniform ivec3 texelDimensions3D;
uniform ivec2 texelDimensions2D;

uniform vec3 r0;
uniform vec3 colour;
uniform vec3 sigma;


vec3 to3DTextureCoordinates(vec2 uv) {
    int width2D = texelDimensions2D[0];
    int height2D = texelDimensions2D[1];
    int width3D = texelDimensions3D[0];
    int height3D = texelDimensions3D[1];
    int length3D = texelDimensions3D[2];
    float wStack = float(width2D)/float(width3D);
    float hStack = float(height2D)/float(height3D);
    float wIndex = floor(uv[1]*hStack)*wStack + floor(uv[0]*wStack);
    return vec3(mod(uv[0]*wStack, 1.0), mod(uv[1]*hStack, 1.0),
                (wIndex + 0.5)/length3D);
}


void main() {
    vec3 uvw = to3DTextureCoordinates(UV);
    float gaussian = exp(-0.5*pow((uvw[0] - r0[0])/sigma[0], 2.0)
                         -0.5*pow((uvw[1] - r0[1])/sigma[1], 2.0)
                         -0.5*pow((uvw[2] - r0[2])/sigma[2], 2.0));
    fragColor = vec4(gaussian*colour, 1.0);

}