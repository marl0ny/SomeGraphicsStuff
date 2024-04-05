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
attribute vec4 inputData;
varying vec4 COLOUR;
#else
in vec4 inputData;
out vec4 COLOUR;
#endif

uniform float viewScale;
uniform vec4 rotation;
uniform float vecScale;

uniform ivec3 texelDimensions3D;

uniform sampler2D vecTex;
uniform sampler2D colTex;


vec3 to3DTextureCoordinates(vec2 uv) {
    int length3D = texelDimensions3D[2];
    float u = mod(uv[0]*float(length3D), 1.0);
    float v = uv[1];
    float w = (floor(uv[0]*float(length3D)) + 0.5)/float(length3D);
    return vec3(u, v, w);
}

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

vec4 project(vec4 x) {
    vec4 y;
    y[0] = x[0]*5.0/(x[2] + 5.0);
    y[1] = x[1]*5.0/(x[2] + 5.0);
    y[2] = x[2];
    y[3] = 1.0;
    return y;
}

void main() {
    // Convert the texture coordinates of the inputData
    //  attribute to 3D texture coordinates
    vec3 r1 = to3DTextureCoordinates(inputData.xy) - vec3(0.5, 0.5, 0.5);
    // Get raw texture data
    vec3 v1 = texture2D(vecTex, inputData.xy).xyz;
    // If vector is greater that a certain length, normalize it
    vec3 v2 = (length(v1) > 0.1)? 0.1*normalize(v1): v1; 
    // vec3 v2 = v1;
    // vec3 v2 = vec3(0.5, 0.5, 0.5);
    vec3 r2 = viewScale*rotate(vec4(r1 + v2*inputData.w*vecScale,
                               1.0), rotation).xyz;
    gl_Position = project(vec4(r2, 1.0));
    COLOUR = texture2D(colTex, inputData.xy);

}