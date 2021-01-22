# version 330 core

in highp vec2 fragTextCoord;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform float dx;
uniform float dy;


void main () {
    /*vec4 u = texture2D(tex2, fragTextCoord + vec2(0.0, dy));
    vec4 d = texture2D(tex2, fragTextCoord + vec2(0.0, -dy));
    vec4 l = texture2D(tex2, fragTextCoord + vec2(-dx, 0.0));
    vec4 r = texture2D(tex2, fragTextCoord + vec2(dx, 0.0));
    vec4 c = texture2D(tex1, fragTextCoord);
    gl_FragColor = (u + d + l + r)/2.0 - c;*/
    if (fragTextCoord.x > dx && fragTextCoord.x < 1.0 - dx && 
        fragTextCoord.y > dx && fragTextCoord.y < 1.0 - dx) {
        vec4 u = textureOffset(tex2, fragTextCoord, ivec2(0, 1));
        vec4 d = textureOffset(tex2, fragTextCoord, ivec2(0, -1));
        vec4 l = textureOffset(tex2, fragTextCoord, ivec2(-1, 0));
        vec4 r = textureOffset(tex2, fragTextCoord, ivec2(1, 0));
        vec4 c = texture2D(tex1, fragTextCoord);
        gl_FragColor = (u + d + l + r)/2.0 - c;
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
