const viewSrc = `#define NAME viewSrc;
precision highp float;
varying highp vec2 fragTexCoord;
uniform sampler2D tex1;
uniform vec4 char1;
uniform vec4 char2;
uniform vec4 char3;
uniform vec4 char4;
uniform vec4 char5;
uniform float mouseX;
uniform float mouseY;

bool isInsideBox(vec2 p, vec2 p01, vec2 p02) {
    float xMax = (p01.x >= p02.x)? p01.x: p02.x;
    float xMin = (p01.x < p02.x)? p01.x: p02.x;
    float yMax = (p01.y >= p02.y)? p01.y: p02.y;
    float yMin = (p01.y < p02.y)? p01.y: p02.y;
    return (p.x >= xMin && p.x <= xMax && 
            p.y >= yMin && p.y <= yMax);
}

void fillCircleAroundMouse(inout vec4 pixelData, vec4 col, vec2 uv) {
    if (((mouseX - uv.x)*(mouseX - uv.x) +
         (mouseY - uv.y)*(mouseY - uv.y)) < 0.0002) {
        pixelData = col;
    }
}

void getCharacter(inout vec4 pixel,
                  vec2 uv, vec2 charXYMin, float charH,
                  vec2 xyMin, vec2 xyMax) {
    float h = abs(xyMax.y - xyMin.y);
    float w = abs(xyMax.x - xyMin.x);
    float m = charH/h;
    float charW = m*w;
    if (uv.x >= charXYMin.x && uv.x <= charXYMin.x + charW
        && uv.y >= charXYMin.y && uv.y <= charXYMin.y + charH) {
        pixel = texture2D(tex1, xyMin + (uv - charXYMin)/m);
    }
}

void main() {
    vec4 pix = vec4(0.0, 0.0, 0.0, 1.0).xyzw;
    getCharacter(pix, fragTexCoord, vec2(0.4, 0.5), 0.1, char1.xy, char1.zw);
    getCharacter(pix, fragTexCoord, vec2(0.475, 0.58), 0.02, char2.xy, char2.zw);
    getCharacter(pix, fragTexCoord, vec2(0.5, 0.5), 0.1, char3.xy, char3.zw);
    getCharacter(pix, fragTexCoord, vec2(0.57, 0.5), 0.1, char4.xy, char4.zw);
    getCharacter(pix, fragTexCoord, vec2(0.64, 0.5), 0.1, char5.xy, char5.zw);
    fillCircleAroundMouse(pix, vec4(0.0, 1.0, 0.0, 1.0), fragTexCoord);
    gl_FragColor = pix;
}`;


const vertexSrc = `#define NAME vertexSrc
attribute vec3 pos;
varying highp vec2 fragTexCoord;

void main() {
    gl_Position = vec4(pos.xyz, 1.0);
    // fragTexCoord = pos.xy;
    // fragTexCoord = vec2(0.5, 0.5) + pos.xy;
    vec2 fontPos = vec2(0.5, 0.5) + vec2(pos.x, pos.y)/2.0;
    fragTexCoord = vec2(fontPos.x, 1.0 - fontPos.y);
}`;


