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

void main() {
    vec4 val = texture2D(tex, UV);
    float x = (val.x >= 0.5)? ((val.x == 0.5)? -0.0: (0.5 - val.x)): val.x;
    float y = (val.y >= 0.5)? ((val.y == 0.5)? -0.0: (0.5 - val.y)): val.y;
    float z = (val.z >= 0.5)? ((val.z == 0.5)? -0.0: (0.5 - val.z)): val.z;
    float w = (val.w >= 0.5)? ((val.w == 0.5)? -0.0: (0.5 - val.w)): val.w;
    fragColor = vec4(x, y, z, w);
}
