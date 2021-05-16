#version 330 core

varying vec3 fragPosition;
varying vec3 fragColour;
varying vec3 fragNormal;
varying vec2 fragTexture;
uniform vec3 lightSource;
uniform vec3 cameraLoc;
uniform vec4 quaternionAxis;
uniform int isTexture;
uniform sampler2D tex;

float getDiffuseLighting(vec3 incomingLight) {
    vec3 light = normalize(incomingLight);
    vec3 orientation = normalize(fragNormal);
    return max(dot(light, orientation), 0.0);
}

float attenuate(float lighting, float dist) {
    return lighting/(1.0 + 0.01*dist + 0.001*dist*dist);
}

float specular(vec3 incomingLight, vec3 cameraPosition) {
    vec3 light = normalize(incomingLight);
    vec3 lightReflection = reflect(light, fragNormal);
    float tmp = dot(lightReflection, normalize(cameraPosition));
    return pow(max(0.0, tmp), 4);
}

void main() {
    vec3 colour;
    vec3 incomingLight = lightSource - fragPosition;
    float lighting = (0.5 + 0.5*getDiffuseLighting(incomingLight)
                      + 0.25*specular(cameraLoc, incomingLight));
    if (!bool(isTexture)) {
        colour = fragColour*lighting;
    } else {
        colour = texture2D(tex, fragTexture).rgb*lighting;
    }
    gl_FragColor = vec4(colour, 1.0);
}
