# version 150 core

varying highp vec2 fragTextCoord;
precision highp float;
uniform sampler2D tex;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform float gridSize;
uniform float brightness;


vec3 complexToColour(float re, float im) {
    float pi = 3.141592653589793;
    float argVal = atan(im, re);
    float maxCol = 1.0;
    float minCol = 50.0/255.0;
    float colRange = maxCol - minCol;
    if (argVal <= pi/3.0 && argVal >= 0.0) {
        return vec3(maxCol,
                    minCol + colRange*argVal/(pi/3.0), minCol);
    } else if (argVal > pi/3.0 && argVal <= 2.0*pi/3.0){
        return vec3(maxCol - colRange*(argVal - pi/3.0)/(pi/3.0),
                    maxCol, minCol);
    } else if (argVal > 2.0*pi/3.0 && argVal <= pi){
        return vec3(minCol, maxCol,
                    minCol + colRange*(argVal - 2.0*pi/3.0)/(pi/3.0));
    } else if (argVal < 0.0 && argVal > -pi/3.0){
        return vec3(maxCol, minCol,
                    minCol - colRange*argVal/(pi/3.0));
    } else if (argVal <= -pi/3.0 && argVal > -2.0*pi/3.0){
        return vec3(maxCol + (colRange*(argVal + pi/3.0)/(pi/3.0)),
                    minCol, maxCol);
    } else if (argVal <= -2.0*pi/3.0 && argVal >= -pi){
        return vec3(minCol,
                    minCol - (colRange*(argVal + 2.0*pi/3.0)/(pi/3.0)), maxCol);
    }
    else {
        return vec3(minCol, maxCol, maxCol);
    }
}


void main() {
    vec2 st = fragTextCoord;
    st.x *= 3.0;
    highp vec4 col;
    if (st.x < 1.0) col = texture(tex, st);
    if (st.x > 1.0 && st.x < 2.0) col = texture(tex2, st);
    else if (st.x > 2.0) col = texture(tex3, st);
    if (st.x < 1.0 || st.x >= 2.0) {
        gl_FragColor = vec4(brightness*abs(col.r)*complexToColour(col.r, 0.0), 1.0);
    } else if (st.x < 2.0) {
        // float a = brightness*(-1.0/((col.g*col.g + col.r*col.r)/
        // (gridSize*gridSize) + 1.0) + 1.0);
        // float a = brightness*log((col.g*col.g + col.r*col.r)/
        // (gridSize*gridSize) + 1.0);
        float a = brightness*sqrt(col.g*col.g + col.r*col.r)/(gridSize);
        gl_FragColor = vec4(a*complexToColour(col.r, col.g), 1.0);
    }
}