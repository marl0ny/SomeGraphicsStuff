# version 330 core

in highp vec2 fragTextCoord;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform float dx;
uniform float dy;

void main () {

    vec4 u = texture(tex2, fragTextCoord + vec2(0.0, dy));
    vec4 d = texture(tex2, fragTextCoord + vec2(0.0, -dy));
    vec4 l = texture(tex2, fragTextCoord + vec2(-dx, 0.0));
    vec4 r = texture(tex2, fragTextCoord + vec2(dx, 0.0));
    vec4 c = texture(tex1, fragTextCoord);
    gl_FragColor = (u + d + l + r)/2.0 - c;

}
