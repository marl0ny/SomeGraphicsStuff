# version 330 core

in highp vec2 fragTextCoord;
uniform float sigma;
uniform float x0;
uniform float y0;
uniform float a;
uniform float r;
uniform float g;
uniform float b;
uniform sampler2D tex;

void main () {
    vec4 prevVal = texture(tex, fragTextCoord);
    float x = fragTextCoord.x - x0;
    float y = fragTextCoord.y - y0;
    float val = a*(1/(sigma*sqrt(2*3.14159)))*exp(-0.5*(x*x + y*y)/(sigma*sigma));
    gl_FragColor = vec4(vec3(r*val, g*val, b*val) + prevVal.rgb, 1.0);
}
