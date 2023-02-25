#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec4 particleTexCoord;
out vec2 UV;
out vec2 FORCE;
out vec4 fragColor;
#define texture2D texture
#else
#define fragColor gl_FragColor
varying highp vec4 particleTexCoord;
varying highp vec2 UV;
varying highp vec2 FORCE;
#endif


uniform bool gradIsComplex;
uniform sampler2D texParticles1;
uniform sampler2D texGradPotential;

vec2 linearTransform(vec2 m, vec2 xy, vec2 b) {
    return vec2(m[0]*xy[0], m[1]*xy[1]) + b;
}

void main() {
    vec4 posVel = texture2D(texParticles1, particleTexCoord.xy);
    vec2 m1 = vec2(0.5, 0.5), b1 = vec2(0.25, 0.25);
    vec4 gradientVal
        = texture2D(texGradPotential, 
                    linearTransform(m1, posVel.xy, b1));
    vec2 f;
    if (gradIsComplex)
        f = vec2(gradientVal[0], gradientVal[2]);
    else
        f = vec2(gradientVal.x, gradientVal.y);
    vec2 m2 = vec2(2.0, 2.0), b2 = vec2(-1.0, -1.0);
    gl_Position = vec4(linearTransform(m2, particleTexCoord.xy, b2),
                       0.0, 1.0);
    UV = posVel.xy;
    FORCE = f;
}
