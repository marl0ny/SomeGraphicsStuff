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

uniform sampler2D tex;
uniform sampler2D boundaryMaskTex;

uniform int index;

uniform int wStack; // Number of xy texture blocks along vertical
uniform int hStack; // Similar to wStack but along the horizontal

uniform vec3 dr;
uniform vec3 dimensions;


vec2 to2DTextureCoordinates(vec3 position) {
    float u = position.x;
    float v = position.y;
    float w = position.z;
    float wRatio = 1.0/float(wStack);
    float hRatio = 1.0/float(hStack);
    int texelLength = wStack*hStack;
    float wIndex = w*float(texelLength) - 0.5;
    vec2 wPosition = vec2(mod(wIndex/float(wStack),
                              float(wStack))/float(wStack),
                          floor(wIndex/float(wStack))/float(hStack));
    return wPosition + vec2(u*wRatio, v*hRatio);
}

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

vec4 centredXDiff4thOrder(sampler2D tex) {
    vec2 dl2 = -2.0*vec2((1.0/float(wStack))*dr[0]/dimensions[0], 0.0);
    vec2 dl1 = -vec2((1.0/float(wStack))*dr[0]/dimensions[0], 0.0);
    vec2 dr1 = vec2((1.0/float(wStack))*dr[0]/dimensions[0], 0.0);
    vec2 dr2 = 2.0*vec2((1.0/float(wStack))*dr[0]/dimensions[0], 0.0);
    float bc = texture2D(boundaryMaskTex,
                         to3DTextureCoordinates(UV).xy)[0];
    float bl2 = texture2D(boundaryMaskTex,
                          to3DTextureCoordinates(UV + dl2).xy)[0];
    float bl1 = texture2D(boundaryMaskTex,
                          to3DTextureCoordinates(UV + dl1).xy)[0];
    float br1 = texture2D(boundaryMaskTex,
                          to3DTextureCoordinates(UV + dr1).xy)[0];
    float br2 = texture2D(boundaryMaskTex,
                          to3DTextureCoordinates(UV + dr2).xy)[0];
    vec4 l2 = bl2*texture2D(tex, UV + dl2);
    vec4 l1 = bl1*texture2D(tex, UV + dl1);
    vec4 r1 = br1*texture2D(tex, UV + dr1);
    vec4 r2 = br2*texture2D(tex, UV + dr2);
    return bc*(l2/12.0 - l1/3.0 + r1/3.0 - r2/12.0)/dr[0];
}

vec4 centredYDiff4thOrder(sampler2D tex) {
    vec2 dd2 = -2.0*vec2(0.0, (1.0/float(hStack))*dr[1]/dimensions[1]);
    vec2 dd1 = -vec2(0.0, (1.0/float(hStack))*dr[1]/dimensions[1]);
    vec2 du1 = vec2(0.0, (1.0/float(hStack))*dr[1]/dimensions[1]);
    vec2 du2 = 2.0*vec2(0.0, (1.0/float(hStack))*dr[1]/dimensions[1]);
    float bc = texture2D(boundaryMaskTex,
                         to3DTextureCoordinates(UV).xy)[0];
    float bd2 = texture2D(boundaryMaskTex,
                          to3DTextureCoordinates(UV + dd2).xy)[0];
    float bd1 = texture2D(boundaryMaskTex,
                          to3DTextureCoordinates(UV + dd1).xy)[0];
    float bu1 = texture2D(boundaryMaskTex,
                          to3DTextureCoordinates(UV + du1).xy)[0];
    float bu2 = texture2D(boundaryMaskTex,
                          to3DTextureCoordinates(UV + du2).xy)[0];
    vec4 d2 = bd2*texture2D(tex, UV + dd2);
    vec4 d1 = bd1*texture2D(tex, UV + dd1);
    vec4 u1 = bu1*texture2D(tex, UV + du1);
    vec4 u2 = bu2*texture2D(tex, UV + du2);
    return bc*(d2/12.0 - d1/3.0 + u1/3.0 - u2/12.0)/dr[1];
}

vec4 centredZDiff4thOrder(sampler2D tex) {
    // TODO
    return vec4(0.0, 0.0, 0.0, 0.0);
}

void main() {
    vec4 dTexdx = centredXDiff4thOrder(tex);
    vec4 dTexdy = centredYDiff4thOrder(tex);
    vec4 dTexdz = centredZDiff4thOrder(tex);
    fragColor = vec4(dTexdx[index], dTexdy[index], dTexdz[index], 1.0); 
}