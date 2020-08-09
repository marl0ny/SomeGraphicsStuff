# version 330 core

in vec2 fragTexture;
in vec3 fragColour;
in vec3 fragPosition;
in vec3 fragNorm;
out vec4 colour;
uniform sampler2D tex;


float getDiffuseLighting(vec3 orientation, vec3 incomingLightRay)  {
    vec3 normalizedLightRay = normalize(incomingLightRay);
    vec3 normalizedOrientation = normalize(orientation);
    float lightVal = max(dot(normalizedOrientation, normalizedLightRay), 0.0);
    return lightVal;
}


float attenuate(float lighting, float distance) {
    return lighting/(1.0 + 0.01*distance + 0.0001*distance*distance);
}


vec3 lightingColour(vec3 colour, 
                     vec3 orientation, vec3 incomingLightRay) {
    vec3 normalizedLightRay = normalize(incomingLightRay);
    vec3 normalizedOrientation = normalize(orientation);
    float lighting = dot(normalizedOrientation, normalizedLightRay);
    vec3 newColor;
    for (int i = 0; i < 3; i++) {
        newColor[i] = ((lighting > 0.0)? lighting: 0.0)*colour[i];
    }
    return newColor;
}


vec4 mergeTextures(vec2 fragTexture) {
    float step = 0.01;
    vec2 textures[9];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            textures[3*i + j] = fragTexture + vec2(step*(i-1), step*(j-1));
        }
    }
    vec4 colourTexture = vec4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < 9; i++) {
        colourTexture += texture(tex, textures[i]);
    }
    colourTexture = colourTexture/9.0;
    colourTexture.a = 1.0;
    return colourTexture;
}


void main () {
    vec3 cameraPosition = vec3(0.0, 0.0, -2.0);
    vec3 cameraToPosition = -fragPosition + cameraPosition;
    vec3 lightSource = vec3(-12.5, 0.0, -25.0);
    // vec3 lightSource = vec3(0.0, 0.0, -25.0);
    vec3 lightVec = normalize(fragPosition - lightSource);
    vec3 reflectionVec = reflect(lightVec, fragNorm);
    vec4 colourTexture = texture(tex, fragTexture);
    float specular = pow(max(0.0, dot(reflectionVec, normalize(cameraToPosition))), 32);
    float lighting = getDiffuseLighting(fragNorm, lightVec) + 0.25 + 0.5*specular;
    // lighting = attenuate(lighting, length(fragPosition - lightSource));
    colour = vec4(colourTexture.rgb*lighting, colourTexture.a);
    // colour = texture(tex, fragTexture);
    // colour = vec4(outColour, 1.0);
}
