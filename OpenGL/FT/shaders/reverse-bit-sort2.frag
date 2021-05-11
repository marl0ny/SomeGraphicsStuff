/*
This did not work at all :(
*/

# version 150 core

varying highp vec2 fragTextCoord;
precision highp float;
uniform int vertOrH;
uniform int n;
// uniform sampler2D lookupTex;
uniform sampler2D tex;

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

/*vec4 getNewVec(vec2 xy) {
    float val = texture2D(lookupTex, vec2(xy.x, xy.y)).r;
   // float val2 = texture2D(tex, vec2(xy.x, xy.y)).r;
    if (xy.x < 0.5 && xy.x > 0.4) {
        return vec4(vec2(val, xy.y), 0.0, 1.0); 
    } else {
        return texture2D(tex, vec2(xy.x, xy.y)); 
    }
}*/

void main() {
    vec2 xy = fragTextCoord;
    if (vertOrH == 0) {
        int i = int(xy.x*n);
        int rev = reverseBitSort2(i, n);
        highp float sampleCoord = float(rev)/float(n-1);
        // gl_FragColor = vec4(xy.x, xy.y, 0.0, 1.0);
        // gl_FragColor = texture2D(tex, xy);
        if (xy.y < 0.5) {
            gl_FragColor = texture2D(tex, vec2(sampleCoord*0.01 + xy.x, xy.y));
        } else {
            gl_FragColor = texture2D(tex, xy);
        }
    } else {
        int rev = reverseBitSort2(n, int(xy.y*n));
        gl_FragColor = texture2D(tex, vec2(xy.x, float(rev)/float(n)));
    }
    /*
    vec4 newVec;
    if (vertOrH == 0) {
        newVec = getNewVec(xy);
        // float val = texture2D(lookupTex, vec2(xy.x, xy.y)).r;
        // newVec = texture2D(lookupTex, vec2(val, xy.y));
        // newVec = texture2D(tex, vec2(0.592157, 0.592157));
        // newVec = texture2D(tex, vec2(1.5, xy.y));
        // newVec = texture2D(tex, vec2(xy.x, xy.y));
        // newVec = vec4(vec2(val, xy.y), texture2D(tex, vec2(xy.x, xy.y)).r, 1.0);
        // newVec = vec4(val, val, val, 1.0);
    } else if (vertOrH == 1){
        float lookupVal = texture2D(lookupTex, vec2(xy.y, xy.x)).r;
        newVec = texture2D(tex, vec2(xy.x, lookupVal));
    } else {
        newVec = texture2D(tex, xy);
    }
    gl_FragColor = newVec;*/
}