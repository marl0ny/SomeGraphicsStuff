# version 330 core

in vec3 position;
in vec3 inColour;
out vec3 outColour;
uniform mat4 rotation;
uniform vec4 qAxis;
uniform float scale;


vec4 quaternionMultiply(vec4 q1, vec4 q2) {
    vec4 q3;
    q3[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    q3[1] = q1[1]*q2[0] + q1[0]*q2[1] + q1[2]*q2[3] - q1[3]*q2[2];
    q3[2] = q1[2]*q2[0] + q1[0]*q2[2] + q1[3]*q2[1] - q1[1]*q2[3];
    q3[3] = q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1];
    return q3;
}


vec3 rotate(vec3 position, vec4 r) {
    vec4 q;
    q[0] = 1.0;
    q[1] = position[0];
    q[2] = position[1];
    q[3] = position[2];
    vec4 qr = quaternionMultiply(q, r);
    r[1] = -r[1];
    r[2] = -r[2];
    r[3] = -r[3];
    vec4 invrqr = quaternionMultiply(r, qr);
    vec3 position2;
    position2[0] = invrqr[1]; 
    position2[1] = invrqr[2];
    position2[2] = invrqr[3];
    return position2;
}


vec3 perspectiveProject(vec3 position) {
    vec3 position2;
    float f = 8.0/(position[2] + 4.0);
    position2[0] = f*position[0];
    position2[1] = f*position[1];
    position2[2] = position[2];
    return position2;
}


vec3 lighting_colour(vec3 colour, 
                     vec3 orientation, vec3 incomingLightRay) {
    vec3 normalizedLightRay = normalize(incomingLightRay);
    vec3 normalizedOrientation = normalize(orientation);
    float lighting = dot(normalizedOrientation, normalizedLightRay);
    vec3 newColor;
    for (int i = 0; i < 3; i++) {
        newColor[i] = ((lighting > 0.0)? (lighting + 0.25): 0.25)*colour[i];
    }
    return newColor;
}


void main () {
    vec3 position2 = rotate(position, qAxis);
    vec3 position3 = perspectiveProject(position2);
    // outColour = lighting_colour(inColour, position3, vec3(1.0, 1.0, 0.0));
    outColour = inColour;
    gl_Position = vec4(scale*position3, 1.0);
}
