# version 330 core

in GS_FS_VERTEX {
    vec3 normal;
    vec3 colour2;
} frag_in;

out vec4 colour;


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
    colour = vec4(0.1*vec3(1.0, 0.25, 0.5) + frag_in.colour2, 1.0);
    /*colour = vec4(
        lighting_colour(colour2, normal, vec3(1.0, 1.0, 0.0)), 1.0
    );*/
}
