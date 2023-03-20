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

uniform vec4 rotation;
uniform sampler2D gradientTex;
uniform sampler2D densityTex;

uniform ivec3 texelDimensions3D;
uniform ivec2 texelDimensions2D;


vec4 quaternionMultiply(vec4 q1, vec4 q2) {
    vec4 q3;
    q3.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
    q3.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y; 
    q3.y = q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z; 
    q3.z = q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x;
    return q3; 
}

vec4 quaternionConjugate(vec4 r) {
    return vec4(-r.x, -r.y, -r.z, r.w);
}

vec4 rotate(vec4 x, vec4 r) {
    vec4 xr = quaternionMultiply(x, r);
    vec4 rInv = quaternionConjugate(r);
    vec4 x2 = quaternionMultiply(rInv, xr);
    x2.w = 1.0;
    return x2; 
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
    vec2 wPosition = vec2(mod(wIndex, wStack)/wStack,
                          floor(wIndex/wStack)/hStack);
    return wPosition + vec2(u*wRatio, v*hRatio);
}

void main() {
    vec3 r = to3DTextureCoordinates(UV);
    vec2 uv2 = to2DTextureCoordinates(r);
    vec3 normal = rotate(vec4(0.0, 0.0, 1.0, 1.0),
                         quaternionConjugate(rotation)).xyz;
    vec3 grad = texture2D(gradientTex, uv2).xyz;
    vec4 density = texture2D(densityTex, uv2);
    vec4 pix = density;
    float dx = 1.0/float(texelDimensions3D[0]);
    float dy = 1.0/float(texelDimensions3D[1]);
    float dz = 1.0/float(texelDimensions3D[2]);
    vec2 zF = to2DTextureCoordinates(vec3(r.x, r.y, r.z + dz));
    vec2 zB = to2DTextureCoordinates(vec3(r.x, r.y, r.z - dz));
    vec2 xF = to2DTextureCoordinates(vec3(r.x + dx, r.y, r.z));
    vec2 xB = to2DTextureCoordinates(vec3(r.x - dx, r.y, r.z));
    vec2 yF = to2DTextureCoordinates(vec3(r.x, r.y + dy, r.z));
    vec2 yB = to2DTextureCoordinates(vec3(r.x, r.y - dy, r.z));
    /* density = (texture2D(densityTex, zF)
                 + texture2D(densityTex, zB)
                + texture2D(densityTex, xF)
                + texture2D(densityTex, xB)
                + texture2D(densityTex, yF)
                + texture2D(densityTex, yB)
                + density)/5.0;*/
    // pix.a = pix.b;
    // lf (length(grad) < 0.0000001) discard;
    if (pix.a < 0.01) discard;
    // fragColor = 4.0*pix;
    float a = dot(normal, normalize(grad));
    if (a <= 0.0) discard;
    fragColor = vec4(a*normalize(density.rgb) , a);
}
