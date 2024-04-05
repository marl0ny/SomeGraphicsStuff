#VERSION_NUMBER_PLACEHOLDER

#if (__VERSION__ >= 330) || (defined(GL_ES) && __VERSION__ >= 300)
#define texture2D texture
#else
#define texture texture2D
#endif

#if (__VERSION__ > 120) || defined(GL_ES)
precision highp float;
#endif

#if __VERSION__ <= 120
attribute vec4 uvIndex;
varying vec2 UV;
#else
in vec4 uvIndex;
out vec2 UV;
#endif

#define quaternion vec4

uniform vec4 debugRotation;
uniform float scale;

uniform ivec3 texelDimensions3D;
uniform ivec2 texelDimensions2D;

/* The attribute or input uvIndex contains the 2D coordinates represetation
of the volume render frame, which is then converted to 3D coordinates
and manipulated using the other uniforms.
It is also directly passed to the fragment shader as the varying or
out variable UV, so that it can be used to sample the volume data which
is store in the 2D texture format.
*/

quaternion mul(quaternion q1, quaternion q2) {
    quaternion q3;
    q3.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
    q3.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y; 
    q3.y = q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z; 
    q3.z = q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x;
    return q3; 
}

quaternion conj(quaternion r) {
    return quaternion(-r.x, -r.y, -r.z, r.w);
}

quaternion rotate(quaternion x, quaternion r) {
    quaternion xr = mul(x, r);
    quaternion rInv = conj(r);
    quaternion x2 = mul(rInv, xr);
    x2.w = 1.0;
    return x2; 
}

vec4 project(vec4 x) {
    vec4 y;
    y[0] = x[0]*5.0/(x[2] + 5.0);
    y[1] = x[1]*5.0/(x[2] + 5.0);
    y[2] = x[2];
    y[3] = 1.0;
    return y;
}

vec3 to3DTextureCoordinates(vec2 uv) {
    int length3D = texelDimensions3D[2];
    float u = mod(uv[0]*float(length3D), 1.0);
    float v = uv[1];
    float w = (floor(uv[0]*float(length3D)) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

void main() {
    UV = uvIndex.xy;
    // gl_Position = vec4(UV, 0.0, 1.0);
    vec4 viewPos = vec4(to3DTextureCoordinates(UV), 1.0)
                   - vec4(0.5, 0.5, 0.5, 0.0);
    gl_Position = project(2.0*scale*rotate(viewPos, debugRotation));
}
