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

vec3 complexToColour(float re, float im) {
    float pi = 3.141592653589793;
    float argVal = atan(im, re);
    float maxCol = 1.0;
    float minCol = 50.0/255.0;
    float colRange = maxCol - minCol;
    if (argVal <= pi/3.0 && argVal >= 0.0) {
        return vec3(maxCol,
                    minCol + colRange*argVal/(pi/3.0), minCol);
    } else if (argVal > pi/3.0 && argVal <= 2.0*pi/3.0){
        return vec3(maxCol - colRange*(argVal - pi/3.0)/(pi/3.0),
                    maxCol, minCol);
    } else if (argVal > 2.0*pi/3.0 && argVal <= pi){
        return vec3(minCol, maxCol,
                    minCol + colRange*(argVal - 2.0*pi/3.0)/(pi/3.0));
    } else if (argVal < 0.0 && argVal > -pi/3.0){
        return vec3(maxCol, minCol,
                    minCol - colRange*argVal/(pi/3.0));
    } else if (argVal <= -pi/3.0 && argVal > -2.0*pi/3.0){
        return vec3(maxCol + (colRange*(argVal + pi/3.0)/(pi/3.0)),
                    minCol, maxCol);
    } else if (argVal <= -2.0*pi/3.0 && argVal >= -pi){
        return vec3(minCol,
                    minCol - (colRange*(argVal + 2.0*pi/3.0)/(pi/3.0)), maxCol);
    }
    else {
        return vec3(minCol, maxCol, maxCol);
    }
}


void main() {
    vec3 uvw = to3DTextureCoordinates(UV);
    float gaussian1 = exp(-0.5*pow((uvw[0] - r0[0])/sigma[0], 2.0)
                          -0.5*pow((uvw[1] - r0[1])/sigma[1], 2.0)
                          -0.5*pow((uvw[2] - r0[2])/sigma[2], 2.0));
    float gaussian2 = exp(-pow((uvw[0] - 4.0*r0[0]/5.0)/sigma[0], 2.0)
                          -pow((uvw[1] - r0[1])/sigma[1], 2.0)
                          -pow((uvw[2] - r0[2]/2.0)/sigma[2], 2.0));
    // float phi = 2.0*3.14159*(2.0*uvw[0] + 10.0*uvw[1] + - 3.0*uvw[2]);
    float phi = 2.0*3.14159*(1.0*uvw[0]);
    // float phi = 3.14159/2.0;
    vec2 complexVal = vec2(cos(phi), sin(phi));
    fragColor = vec4((gaussian1 + gaussian2)
                     *complexToColour(complexVal.r, complexVal.g),
                     gaussian1 + gaussian2);
}