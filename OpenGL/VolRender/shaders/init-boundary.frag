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
uniform vec3 dr;
uniform vec3 dimensions;



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
    if (uvw[0] < dr[0]/dimensions[0] ||
        uvw[1] < dr[1]/dimensions[1] ||
        uvw[2] < dr[2]/dimensions[2] ||
        uvw[0] > (1.0 - dr[0]/dimensions[0]) ||
        uvw[1] > (1.0 - dr[1]/dimensions[1]) ||
        uvw[2] > (1.0 - dr[2]/dimensions[2])) {
        fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
}