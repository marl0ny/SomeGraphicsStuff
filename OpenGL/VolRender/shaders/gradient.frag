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

uniform ivec3 texelDimensions3D;
uniform ivec2 texelDimensions2D;

uniform vec3 dr; // Spartial step sizes
uniform vec3 dimensions3D; // Dimensions of simulation


vec2 to2DTextureCoordinates(vec3 position) {
    int width2D = texelDimensions2D[0];
    int height2D = texelDimensions2D[1];
    int width3D = texelDimensions3D[0];
    int height3D = texelDimensions3D[1];
    int length3D = texelDimensions3D[2];
    float wStack = float(width2D)/float(width3D);
    float hStack = float(height2D)/float(height3D);
    float u = position.x;
    float v = position.y;
    float w = position.z;
    float wRatio = 1.0/wStack;
    float hRatio = 1.0/hStack;
    float wIndex = w*float(length3D) - 0.5;
    vec2 wPosition = vec2(mod(wIndex ,wStack)/wStack,
                          floor(wIndex/wStack)/hStack);
    return wPosition + vec2(u*wRatio, v*hRatio);
}

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
                (wIndex + 0.5)/float(length3D));
}

vec4 centredXDiff4thOrder(sampler2D tex) {
    int width2D = texelDimensions2D[0];
    int height2D = texelDimensions2D[1];
    int width3D = texelDimensions3D[0];
    int height3D = texelDimensions3D[1];
    int length3D = texelDimensions3D[2];
    float wStack = float(width2D)/float(width3D);
    vec2 dl2 = -2.0*vec2((1.0/wStack)*dr[0]/dimensions3D[0], 0.0);
    vec2 dl1 = -vec2((1.0/wStack)*dr[0]/dimensions3D[0], 0.0);
    vec2 dr1 = vec2((1.0/wStack)*dr[0]/dimensions3D[0], 0.0);
    vec2 dr2 = 2.0*vec2((1.0/wStack)*dr[0]/dimensions3D[0], 0.0);

    /* float bc = texture2D(boundaryMaskXYTex,
                         to3DTextureCoordinates(UV).xy)[0];
    float bl2 = texture2D(boundaryMaskXYTex,
                          to3DTextureCoordinates(UV + dl2).xy)[0];
    float bl1 = texture2D(boundaryMaskXYTex,
                          to3DTextureCoordinates(UV + dl1).xy)[0];
    float br1 = texture2D(boundaryMaskXYTex,
                          to3DTextureCoordinates(UV + dr1).xy)[0];
    float br2 = texture2D(boundaryMaskXYTex,
                          to3DTextureCoordinates(UV + dr2).xy)[0];*/

    float bc = texture2D(boundaryMaskTex, UV)[0];
    float bl2 = texture2D(boundaryMaskTex, UV + dl2)[0];
    float bl1 = texture2D(boundaryMaskTex, UV + dl1)[0];
    float br1 = texture2D(boundaryMaskTex, UV + dr1)[0];
    float br2 = texture2D(boundaryMaskTex, UV + dr2)[0];
    vec4 l2 = bl2*texture2D(tex, UV + dl2);
    vec4 l1 = bl1*texture2D(tex, UV + dl1);
    vec4 r1 = br1*texture2D(tex, UV + dr1);
    vec4 r2 = br2*texture2D(tex, UV + dr2);
    return bc*(l2/12.0 - l1/3.0 + r1/3.0 - r2/12.0)/dr[0];
}

vec4 centredYDiff4thOrder(sampler2D tex) {
    int width2D = texelDimensions2D[0];
    int height2D = texelDimensions2D[1];
    int width3D = texelDimensions3D[0];
    int height3D = texelDimensions3D[1];
    int length3D = texelDimensions3D[2];
    float hStack = float(height2D)/float(height3D);
    vec2 dd2 = -2.0*vec2(0.0, (1.0/hStack)*dr[1]/dimensions3D[1]);
    vec2 dd1 = -vec2(0.0, (1.0/hStack)*dr[1]/dimensions3D[1]);
    vec2 du1 = vec2(0.0, (1.0/hStack)*dr[1]/dimensions3D[1]);
    vec2 du2 = 2.0*vec2(0.0, (1.0/hStack)*dr[1]/dimensions3D[1]);

    /* float bc = texture2D(boundaryMaskXYTex,
                         to3DTextureCoordinates(UV).xy)[0];
    float bd2 = texture2D(boundaryMaskXYTex,
                          to3DTextureCoordinates(UV + dd2).xy)[0];
    float bd1 = texture2D(boundaryMaskXYTex,
                          to3DTextureCoordinates(UV + dd1).xy)[0];
    float bu1 = texture2D(boundaryMaskXYTex,
                          to3DTextureCoordinates(UV + du1).xy)[0];
    float bu2 = texture2D(boundaryMaskXYTex,
                          to3DTextureCoordinates(UV + du2).xy)[0];*/

    float bc = texture2D(boundaryMaskTex, UV)[0];
    float bd2 = texture2D(boundaryMaskTex, UV + dd2)[0];
    float bd1 = texture2D(boundaryMaskTex, UV + dd1)[0];
    float bu1 = texture2D(boundaryMaskTex, UV + du1)[0];
    float bu2 = texture2D(boundaryMaskTex, UV + du2)[0];
    vec4 d2 = bd2*texture2D(tex, UV + dd2);
    vec4 d1 = bd1*texture2D(tex, UV + dd1);
    vec4 u1 = bu1*texture2D(tex, UV + du1);
    vec4 u2 = bu2*texture2D(tex, UV + du2);
    return bc*(d2/12.0 - d1/3.0 + u1/3.0 - u2/12.0)/dr[1];
}

vec4 centredZDiff4thOrder(sampler2D tex) {
    vec3 xyz = to3DTextureCoordinates(UV).xyz;
    float x = xyz.x, y = xyz.y, z = xyz.z;
    float zf1 = z + dr[2]/dimensions3D[2];
    float zf2 = z + 2.0*dr[2]/dimensions3D[2];
    float zb1 = z - dr[2]/dimensions3D[2];
    float zb2 = z - 2.0*dr[2]/dimensions3D[2];

    /* float bc = texture2D(boundaryMaskTex, vec2(z, 0.5))[0];
    float bf2 = texture2D(boundaryMaskTex, vec2(zf2, 0.5))[0];
    float bf1 = texture2D(boundaryMaskTex, vec2(zf1, 0.5))[0];
    float bb1 = texture2D(boundaryMaskTex, vec2(zb1, 0.5))[0];
    float bb2 = texture2D(boundaryMaskTex, vec2(zb2, 0.5))[0];*/

    float bc = texture2D(boundaryMaskTex, UV)[0];
    float bf2 = texture2D(boundaryMaskTex,
                          to2DTextureCoordinates(vec3(x, y, zf2)))[0];
    float bf1 = texture2D(boundaryMaskTex,
                          to2DTextureCoordinates(vec3(x, y, zf2)))[0];
    float bb1 = texture2D(boundaryMaskTex,
                          to2DTextureCoordinates(vec3(x, y, zb1)))[0];
    float bb2 = texture2D(boundaryMaskTex,
                          to2DTextureCoordinates(vec3(x, y, zb2)))[0];
    vec4 f2 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zf2)));
    vec4 f1 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zf1)));
    vec4 b1 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zb1)));
    vec4 b2 = texture2D(tex, to2DTextureCoordinates(vec3(x, y, zb2)));
    
    /*// Samples at neighbouring slices
    vec2 df1InRow = vec2(1.0/float(wStack), 0.0);
    vec2 df2InRow = 2.0*vec2(1.0/float(wStack), 0.0);
    vec2 db1InRow = -vec2(1.0/float(wStack), 0.0);
    vec2 db2InRow = -2.0*vec2(1.0/float(wStack), 0.0);
    vec4 f1InRow = texture2D(tex, UV + df1InRow);
    vec4 f2InRow = texture2D(tex, UV + df2InRow);
    vec4 b1InRow = texture2D(tex, UV + db1InRow);
    vec4 b2InRow = texture2D(tex, UV + db2InRow);
    // Samples at previous and next row of slices
    vec2 df2NextRow = vec2(-float(wStack-2)/float(wStack), 1.0/float(hStack));
    vec2 df1NextRow = vec2(-float(wStack-1)/float(wStack), 1.0/float(hStack));
    vec2 db1PrevRow = vec2(float(wStack-1)/float(wStack), -1.0/float(hStack));
    vec2 db2PrevRow = vec2(float(wStack-2)/float(wStack), -1.0/float(hStack));
    vec4 f1NextRow = texture2D(tex, UV + df1NextRow);
    vec4 f2NextRow = texture2D(tex, UV + df2NextRow);
    vec4 b1PrevRow = texture2D(tex, UV + db1PrevRow);
    vec4 b2PrevRow = texture2D(tex, UV + db2PrevRow);
    float bc = texture2D(boundaryMaskZTex, )[0];
    float bf2 = texture2D(boundaryMaskZTex, )[0];
    float bf1 = texture2D(boundaryMaskZTex, )[0];
    float bb1 = texture2D(boundaryMaskZTex, )[0];
    float bb2 = texture2D(boundaryMaskZTex, )[0];
    vec4 f1 = bf1*(f1InRow + f1NextRow);
    vec4 f2 = bf2*(f2InRow + f2NextRow);
    vec4 b1 = bb1*(b1InRow + b1PrevRow);
    vec4 b2 = bb2*(b2InRow + b2PrevRow);*/

    return bc*(b2/12.0 - b1/3.0 + f1/3.0 - f2/12.0)/dr[2];
}

void main() {
    vec4 dTexdx = centredXDiff4thOrder(tex);
    vec4 dTexdy = centredYDiff4thOrder(tex);
    vec4 dTexdz = centredZDiff4thOrder(tex);
    fragColor = vec4(dTexdx[index], dTexdy[index], dTexdz[index], 1.0); 
}