#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec4 uvIndex;
out vec3 POSITION;
#define texture2D texture
#else
attribute vec4 uvIndex;
attribute vec3 POSITION;
#endif

uniform float scale;
uniform vec4 rotation;

uniform ivec3 renderTexelDimensions3D;
uniform ivec2 renderTexelDimensions2D;


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
    int width2D = renderTexelDimensions2D[0];
    int height2D = renderTexelDimensions2D[1];
    int width3D = renderTexelDimensions3D[0];
    int height3D = renderTexelDimensions3D[1];
    int length3D = renderTexelDimensions3D[2];
    float wStack = float(width2D)/float(width3D);
    float hStack = float(height2D)/float(height3D);
    float wIndex = floor(uv[1]*hStack)*wStack + floor(uv[0]*wStack);
    return vec3(mod(uv[0]*wStack, 1.0), mod(uv[1]*hStack, 1.0),
                (wIndex + 0.5)/float(length3D));
}


void main() {
    vec2 uv = uvIndex.xy;
    vec4 viewPos = vec4(to3DTextureCoordinates(uv), 1.0)
                   - vec4(0.5, 0.5, 0.5, 0.0);
    // 3D coordinates used in the fragment shader when sampling from the
    // 3D texture. If no scaling or rotation is done, then
    // 0 <= POSITION.x < 1, etc.
    POSITION = (scale*rotate(viewPos, quaternionConjugate(rotation))
                             + vec4(0.5, 0.5, 0.5, 0.0)).xyz;
    // Write to the frame buffer as a 2D texture. Scale things so that
    // it covers the entire frame buffer i.e.
    // -1.0 < x < 1.0 etc.
    gl_Position = 2.0*(vec4(uv, 0.0, 0.5) - vec4(0.5, 0.5, 0.0 ,0.0));
}
