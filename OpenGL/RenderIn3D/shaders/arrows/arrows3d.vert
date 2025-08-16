#if __VERSION__ <= 120
attribute vec4 position;
varying vec2 UV;
#else
in vec4 position;
out vec2 UV;
#endif

#if (__VERSION__ >= 330) || (defined(GL_ES) && __VERSION__ >= 300)
#define texture2D texture
#else
#define texture texture2D
#endif

#if (__VERSION__ > 120) || defined(GL_ES)
precision highp float;
#endif

#define quaternion vec4

uniform sampler2D vecTex;
uniform float arrowScale;
uniform float maxLength;
uniform quaternion rotation;
uniform vec3 translate;
uniform float scale;
uniform ivec2 screenDimensions;
uniform ivec2 texelDimensions2D;
uniform ivec3 texelDimensions3D;


quaternion mul(quaternion q1, quaternion q2) {
    quaternion q3;
    q3.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
    q3.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y; 
    q3.y = q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z; 
    q3.z = q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x;
    return q3; 
}

quaternion conj(quaternion q) {
    return quaternion(-q.x, -q.y, -q.z, q.w);
}

quaternion rotate(quaternion x, quaternion r) {
    return quaternion(mul(conj(r), mul(x, r)).xyz, 1.0);
}

vec4 project(vec4 x) {
    vec4 y;
    y[0] = x[0]*4.0/(x[2] + 4.0);
    y[1] = float(screenDimensions[0])/float(screenDimensions[1])
            *x[1]*4.0/(x[2] + 4.0);
    y[2] = x[2]/4.0;
    y[3] = 1.0;
    return y;
}

vec2 transform(vec2 r, float ratio, float scale) {
    float h = sqrt(1.0 + ratio*ratio);
    float c = 1.0/h, s = ratio/h;
    return  scale*vec2(
        r.x*c - r.y*s, 
        r.x*s + r.y*c
    );
}

vec3 to3DTextureCoordinates(vec2 uv) {
    int width3D = texelDimensions3D[0];
    int height3D = texelDimensions3D[1];
    int length3D = texelDimensions3D[2];
    int width2D = texelDimensions2D[0];
    int height2D = texelDimensions2D[1];
    float wStack = float(width2D)/float(width3D);
    float hStack = float(height2D)/float(height3D);
    float u = mod(uv[0]*wStack, 1.0);
    float v = mod(uv[1]*hStack, 1.0);
    float w = (floor(uv[1]*hStack)*wStack
               + floor(uv[0]*wStack) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

void main() {
    UV = position.xy;
    vec3 uvw = to3DTextureCoordinates(UV);
    float ratio = position[2];
    float arrowScale = position[3];
    vec3 direction = texture2D(vecTex, UV).xyz;
    direction.z = -direction.z;
    if (length(direction) > maxLength)
        direction = normalize(direction)*maxLength;
    vec3 arrowRelPoint = arrowScale*direction - vec3(0.0, 0.0, 0.1);
    vec4 position2 = scale*(vec4(uvw - vec3(0.5), 0.0) 
                            + vec4(arrowRelPoint, 0.0));
    vec4 finalPosition = rotate(position2, rotation) + vec4(translate, 0.0);
    gl_Position = project(finalPosition);
    
}