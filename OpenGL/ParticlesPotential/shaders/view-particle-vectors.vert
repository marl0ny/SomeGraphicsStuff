#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec4 particleTexCoord;
out vec2 UV;
out vec4 fragColor;
#define texture2D texture
#else
#define fragColor gl_FragColor
varying highp vec4 particleTexCoord;
varying highp vec2 UV;
#endif

uniform sampler2D posVelTex;
uniform sampler2D vecTex;
uniform ivec2 vecIndex;
uniform float vecScale;

void main() {
    vec4 posVel = texture2D(posVelTex, particleTexCoord.xy);
    vec4 vector4 = texture2D(vecTex, particleTexCoord.xy);
    vec2 vector2;
    if (vecIndex[0] == 0 && vecIndex[1] == 1)
        vector2 = vector4.xy;
    else if (vecIndex[0] == 0 && vecIndex[1] == 2)
        vector2 = vector4.xz;
    else
        vector2 = vector4.zw;
    gl_Position = vec4(posVel.xy + vecScale*particleTexCoord.a*vector2,
                       0.0, 1.0);
}