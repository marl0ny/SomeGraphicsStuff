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

uniform vec4 w00;
uniform vec4 w10;
uniform vec4 w01; 
uniform vec4 w11;

void main() {
    fragColor = mix(mix(w00, w10, UV.x), mix(w01, w11, UV.x), UV.y);
}
