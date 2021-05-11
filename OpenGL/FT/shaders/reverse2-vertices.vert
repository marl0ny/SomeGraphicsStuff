# version 150 core

attribute vec3 position;
varying highp vec2 fragTextCoord;
uniform int n;


int reverseBitSort2(int i, int n) {
    int u = 1;
    int d = n/2;
    int rev = 0;
    while (u < n) {
        rev += d*((i&u)/u);
        u *= 2;
        d = d/2;
    }
    return rev;
}


void main () {
    float x = position.x/2.0 + 0.5;
    float y = position.y/2.0 + 0.5;
    int i = int(x*n);
    int rev = reverseBitSort2(i, n);
    highp float xPrime = float(rev)/float(n-1);
    fragTextCoord = vec2(x-0.5, y);
    gl_Position = vec4(position.xyz, 1.0);
}
