#define NAME vertexSrc
attribute vec3 pos;
varying highp vec2 fragTexCoord;

void main() {
    gl_Position = vec4(pos.xyz, 1.0);
    // fragTexCoord = pos.xy;
    // fragTexCoord = vec2(0.5, 0.5) + pos.xy;
    vec2 fontPos = vec2(0.5, 0.5) + vec2(pos.x, pos.y)/2.0;
    fragTexCoord = vec2(fontPos.x, 1.0 - fontPos.y);
}