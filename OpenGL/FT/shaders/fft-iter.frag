/*
TODO: Currently all the fft operations are done on the cpu instead
of the gpu. Also figure out how to do it on the gpu and compare it 
with the cpu inplementation.
*/

#version 150 core

precision highp float;
varying highp vec2 fragTextCoord;
uniform sampler2D tex;
uniform float blockSize;
uniform int isVertical;
uniform float sign;
uniform float invSize;
float tau = 6.283185307179586;

void main() {
    vec2 xy = fragTextCoord;
    float eps = 1.0/512.0;
    if (isVertical == 0) {
        float x = xy.x;
        float val = mod(x, blockSize);
        if (val <= blockSize/2.0) {
            vec3 even = texture2D(tex, xy).rgb;
            vec3 odd = texture2D(tex, 
                vec2(x + blockSize/2.0, xy.y)).rgb;
            float phi = -sign*tau*(x)/(blockSize);
            vec3 odd2 = vec3(
                odd.r*cos(phi) - odd.g*sin(phi),
                odd.r*sin(phi) + odd.g*cos(phi),
                0.0);
            gl_FragColor = vec4(invSize*(even + odd2), 1.0);
        } else {
            vec3 even = texture2D(tex, 
                vec2(x - blockSize/2.0, xy.y)).rgb;
            vec3 odd = texture2D(tex, xy).rgb;
            float phi = -sign*tau*(x - blockSize/2.0)/(blockSize);
            vec3 odd2 = vec3(
                odd.r*cos(phi) - odd.g*sin(phi),
                odd.r*sin(phi) + odd.g*cos(phi),
                0.0);
            gl_FragColor = vec4(invSize*(even - odd2), 1.0);
        }
    } else {
        float y = xy.y;
        float val = mod(y, blockSize);
        if (val <= blockSize/2.0) {
            vec3 even = texture2D(tex, xy).rgb;
            vec3 odd = texture2D(tex, 
                vec2(xy.x, y+blockSize/2.0)).rgb;
            float phi = -sign*tau*(y)/blockSize;
            vec3 odd2 = vec3(
                odd.r*cos(phi) - odd.g*sin(phi),
                odd.r*sin(phi) + odd.g*cos(phi),
                0.0);
            gl_FragColor = vec4(invSize*(even + odd2), 1.0);
        } else {
            vec3 even = texture2D(tex, 
                vec2(xy.x, y-blockSize/2.0)).rgb;
            vec3 odd = texture2D(tex, xy).rgb;
            float phi = -sign*tau*(y - blockSize/2.0)/blockSize;
            vec3 odd2 = vec3(
                odd.r*cos(phi) - odd.g*sin(phi),
                odd.r*sin(phi) + odd.g*cos(phi),
                0.0);
            gl_FragColor = vec4(invSize*(even - odd2), 1.0);
        }
    }
}