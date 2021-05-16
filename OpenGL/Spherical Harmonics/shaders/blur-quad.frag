#version 330 core

varying vec2 fragTextCoord;
uniform sampler2D tex;
uniform int width;
uniform int height;

vec3 getBlurredPixel() {
    float gaussianKernel[49];
    gaussianKernel[0] = 8.831959852548556e-13;
    gaussianKernel[1] = 2.1444141378790494e-07;
    gaussianKernel[2] = 5.353209030595414e-05;
    gaussianKernel[3] = 0.00024476077204550875;
    gaussianKernel[4] = 5.353209030595414e-05;
    gaussianKernel[5] = 2.1444141378790494e-07;
    gaussianKernel[6] = 8.831959852548556e-13;
    gaussianKernel[7] = 2.1444141378790494e-07;
    gaussianKernel[8] = 0.000953635280585937;
    gaussianKernel[9] = 0.02159638660527522;
    gaussianKernel[10] = 0.044368333871782226;
    gaussianKernel[11] = 0.02159638660527522;
    gaussianKernel[12] = 0.000953635280585937;
    gaussianKernel[13] = 2.1444141378790494e-07;
    gaussianKernel[14] = 5.353209030595414e-05;
    gaussianKernel[15] = 0.02159638660527522;
    gaussianKernel[16] = 0.1158766211045931;
    gaussianKernel[17] = 0.14730805612132933;
    gaussianKernel[18] = 0.1158766211045931;
    gaussianKernel[19] = 0.02159638660527522;
    gaussianKernel[20] = 5.353209030595414e-05;
    gaussianKernel[21] = 0.00024476077204550875;
    gaussianKernel[22] = 0.044368333871782226;
    gaussianKernel[23] = 0.14730805612132933;
    gaussianKernel[24] = 0.15957691216057307;
    gaussianKernel[25] = 0.14730805612132933;
    gaussianKernel[26] = 0.044368333871782226;
    gaussianKernel[27] = 0.00024476077204550875;
    gaussianKernel[28] = 5.353209030595414e-05;
    gaussianKernel[29] = 0.02159638660527522;
    gaussianKernel[30] = 0.1158766211045931;
    gaussianKernel[31] = 0.14730805612132933;
    gaussianKernel[32] = 0.1158766211045931;
    gaussianKernel[33] = 0.02159638660527522;
    gaussianKernel[34] = 5.353209030595414e-05;
    gaussianKernel[35] = 2.1444141378790494e-07;
    gaussianKernel[36] = 0.000953635280585937;
    gaussianKernel[37] = 0.02159638660527522;
    gaussianKernel[38] = 0.044368333871782226;
    gaussianKernel[39] = 0.02159638660527522;
    gaussianKernel[40] = 0.000953635280585937;
    gaussianKernel[41] = 2.1444141378790494e-07;
    gaussianKernel[42] = 8.831959852548556e-13;
    gaussianKernel[43] = 2.1444141378790494e-07;
    gaussianKernel[44] = 5.353209030595414e-05;
    gaussianKernel[45] = 0.00024476077204550875;
    gaussianKernel[46] = 5.353209030595414e-05;
    gaussianKernel[47] = 2.1444141378790494e-07;
    gaussianKernel[48] = 8.831959852548556e-13;
    vec3 pixVal = vec3(0.0, 0.0, 0.0);
    float x = fragTextCoord.x;
    float y = fragTextCoord.y;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            int p = i - 3;
            int q = j - 3;
            vec2 offset = vec2(x+float(p)/float(width), 
                               y+float(q)/float(height));
            pixVal += gaussianKernel[i*7 + j]*texture2D(tex, offset).rgb;
        }
    }
    return pixVal;
}

void main() {
    highp vec4 col = vec4(getBlurredPixel(), 1.0);
    gl_FragColor = col;
}
