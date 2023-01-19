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

void main() {
    vec3 normal = rotate(vec4(0.0, 0.0, 1.0, 1.0),
                         quaternionConjugate(rotation)).xyz;
    vec3 grad = texture2D(gradientTex, UV).xyz;
    vec4 density = texture2D(densityTex, UV);
    vec4 pix = density;
    pix.a = pix.b;
    // lf (length(grad) < 0.0000001) discard;
    if (pix.a < 0.01) discard;
    // fragColor = 4.0*pix;
    float a = dot(normal, normalize(grad));
    fragColor = vec4(normalize(grad) , sqrt(abs(a)));
}
