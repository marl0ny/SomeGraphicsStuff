# version 150 core

varying highp vec2 fragTextCoord;
precision highp float;
uniform int type;
uniform float sigma;
uniform float k;
uniform float j;
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
    float val;
    float PI = 3.141592653589793;
    if (type == 0) {
        val = a*(1/(sigma*sqrt(2*PI)))*exp(-0.5*(x*x + y*y)/(sigma*sigma));
    } else if (type == 1) {
        val = a*sin(2.0*PI*sqrt(k*k*x*x + j*j*y*y))/sqrt(x*x + y*y);
    } else if (type == 2) {
        val = a*(1/(sigma*sqrt(2*PI)))*exp(-0.5*(x*x + y*y)/(sigma*sigma))
              *sin(2.0*PI*sqrt(k*k*x*x + j*j*y*y));
    } else if (type == 3) {
        val = a*sin(2.0*PI*sqrt(k*k*x*x + j*j*y*y));
    } else if (type == 4) {
        float d = sqrt(k*k*x*x + j*j*y*y);
        val = (d < sigma)? a: 0.0;
    } else if (type == 5) {
        val = a*(1/(sigma*sqrt(2*PI)))*exp(-0.5*(x*x + y*y)/(sigma*sigma))*
                sin(2.0*PI*(k*x + j*y));
    } else if (type == 6) {
        val = a*cos(2.0*PI*(k*x + j*y));
    } else if (type == 7) {
        val = 0.0;
    }
    gl_FragColor = vec4(vec3(r*val, g*val, b*val) + prevVal.rgb, 1.0);
}
