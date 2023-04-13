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

#define complex vec2
#define complex2 vec4

uniform sampler2D tex;

void main() {
    complex2 val = texture2D(tex, UV);
    fragColor = complex2(complex(val.x, -val.y),
                         complex(val.z, -val.w));
}