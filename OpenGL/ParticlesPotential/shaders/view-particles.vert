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

uniform sampler2D tex;


void main() {
    vec4 posVel = texture2D(tex, particleTexCoord.xy);
    gl_Position = vec4(posVel.xy, 0.0, 1.0);
}
