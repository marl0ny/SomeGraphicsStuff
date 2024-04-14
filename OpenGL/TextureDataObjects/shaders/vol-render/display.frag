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
varying vec2 UV;
#define fragColor gl_FragColor
#else
in vec2 UV;
out vec4 fragColor;
#endif

#define quaternion vec4

uniform vec4 rotation;
uniform sampler2D gradientTex;
uniform sampler2D densityTex;

uniform ivec3 texelDimensions3D;
uniform ivec2 texelDimensions2D;

/* The variable UV from the previous shader contains 
the 2D texture coordinate representation of the volume render.
It is 2D so that the gradient and density uniform textures are
properly sampled. These sampled gradient and density data points are used
together to determine how the pixel should be displayed.

This corresponds to the shading step as given on the Wikipedia
page for Volume ray casting.

References:

Volume ray casting - Wikipedia
https://en.wikipedia.org/wiki/Volume_ray_casting

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

vec3 to3DTextureCoordinates(vec2 uv) {
    int length3D = texelDimensions3D[2];
    float u = mod(uv[0]*float(length3D), 1.0);
    float v = uv[1];
    float w = (floor(uv[0]*float(length3D)) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

vec2 to2DTextureCoordinates(vec3 position) {
    int width2D = texelDimensions2D[0];
    int width3D = texelDimensions3D[0];
    int length3D = texelDimensions3D[2];
    float xIndex = float(width3D)*mod(position.x, 1.0);
    float zIndex = float(length3D)*mod(position.z, 1.0);
    float uIndex = floor(zIndex)*float(width3D) + xIndex; 
    return vec2(uIndex/float(width2D), position.y);
}

// void main() {
//     vec3 r = to3DTextureCoordinates(UV);
//     vec2 uv2 = to2DTextureCoordinates(r);
//     fragColor = vec4(r.z, r.z, r.z, length(r));
// }


void main() {
    vec3 r = to3DTextureCoordinates(UV);
    vec2 uv2 = to2DTextureCoordinates(r);
    vec3 normal = rotate(quaternion(0.0, 0.0, 1.0, 1.0),
                         conj(rotation)).xyz;
    vec3 grad = texture2D(gradientTex, uv2).xyz;
    vec4 density = texture2D(densityTex, uv2);
    vec4 pix = density;
    float dx = 1.0/float(texelDimensions3D[0]);
    float dy = 1.0/float(texelDimensions3D[1]);
    float dz = 1.0/float(texelDimensions3D[2]);
    /* vec2 zF = to2DTextureCoordinates(vec3(r.x, r.y, r.z + dz));
    vec2 zB = to2DTextureCoordinates(vec3(r.x, r.y, r.z - dz));
    vec2 xF = to2DTextureCoordinates(vec3(r.x + dx, r.y, r.z));
    vec2 xB = to2DTextureCoordinates(vec3(r.x - dx, r.y, r.z));
    vec2 yF = to2DTextureCoordinates(vec3(r.x, r.y + dy, r.z));
    vec2 yB = to2DTextureCoordinates(vec3(r.x, r.y - dy, r.z));
    density = (texture2D(densityTex, zF)
                 + texture2D(densityTex, zB)
                 + texture2D(densityTex, xF)
                 + texture2D(densityTex, xB)
                + texture2D(densityTex, yF)
                + texture2D(densityTex, yB)
                 + density);*/
    // pix.a = pix.b;
    // lf (length(grad) < 0.0000001) discard;
    if (pix.a < 0.05) discard;
    // fragColor = 4.0*pix;
    float a = dot(normal, normalize(grad));
    if (a <= 0.0) discard;
    fragColor = vec4(a*normalize(density.rgb), 0.15*a);
    // fragColor = vec4(1.0);
}