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

uniform sampler2D tex;

void main() {
    vec4 val = texture2D(tex, UV);
    float x = (val.x >= 0.5)? ((val.x == 0.5)? -0.0: (0.5 - val.x)): val.x;
    float y = (val.y >= 0.5)? ((val.y == 0.5)? -0.0: (0.5 - val.y)): val.y;
    float z = (val.z >= 0.5)? ((val.z == 0.5)? -0.0: (0.5 - val.z)): val.z;
    float w = (val.w >= 0.5)? ((val.w == 0.5)? -0.0: (0.5 - val.w)): val.w;
    fragColor = vec4(x, y, z, w);
}
