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

uniform sampler2D tex;
uniform float n;

void main() {
    vec4 val = texture2D(tex, UV);
    fragColor = vec4(pow(abs(val.x), n), pow(abs(val.y), n),
                     pow(abs(val.z), n), pow(abs(val.w), n));
}