# version 330 core


in vec3 position;
out highp vec2 fragTextCoord;
uniform float angle;
uniform float scale;


vec3 rotate(float angle, vec3 vect) {
    return vec3(
        vect.x*cos(angle) + vect.y*sin(angle),
        -vect.x*sin(angle) + vect.y*cos(angle),
        vect.z
    );
}

void main () {
    vec3 scaledPosition = scale*position;
    vec3 rotatedPosition = rotate(angle, scaledPosition);
    fragTextCoord = vec2(0.5, 0.5) + vec2(rotatedPosition[0], rotatedPosition[1])/2.0;
    gl_Position = vec4(position.xyz, 1.0);
}
