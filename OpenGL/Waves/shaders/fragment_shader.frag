# version 330 core

in highp vec2 fragTextCoord;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D inputTex;
uniform int isViewFrame;


void main () {
    if (isViewFrame == 0) {
        highp float dx = 1.0/1024.0;
        if (fragTextCoord.x > dx && fragTextCoord.x < 1.0 - dx && fragTextCoord.y > dx && fragTextCoord.y < 1.0 - dx) {
            gl_FragColor = (textureOffset(tex2, fragTextCoord, ivec2(0, 1)) + textureOffset(tex2, fragTextCoord, ivec2(0, -1))
                            + textureOffset(tex2, fragTextCoord, ivec2(1, 0)) + textureOffset(tex2, fragTextCoord, ivec2(-1, 0)))/2.0
                            - texture2D(tex1, fragTextCoord);
        } else {
            gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    } else {
        highp vec4 col = texture(tex1, fragTextCoord);
        highp vec4 col2 = texture(inputTex, fragTextCoord);
        gl_FragColor = vec4(col.r+col2.r, col.r+col2.r, -col.r-col2.r, 1.0);
    }
}
