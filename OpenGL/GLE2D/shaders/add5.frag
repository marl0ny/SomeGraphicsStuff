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

#define complex2 vec4

uniform float s0;
uniform sampler2D tex0;
uniform float s1;
uniform sampler2D tex1;
uniform float s2;
uniform sampler2D tex2;
uniform float s3;
uniform sampler2D tex3;
uniform float s4;
uniform sampler2D tex4;

void main() {
    fragColor = (+ s0*texture2D(tex0, UV)
                 + s1*texture2D(tex1, UV) + s2*texture2D(tex2, UV)
                 + s3*texture2D(tex3, UV) + s4*texture2D(tex4, UV));
                 
}