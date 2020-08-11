# version 330 core


in vec3 position;
in vec3 inColour;
in vec3 normal;
in vec2 texture;
out vec2 fragTexture;
out vec3 fragPosition;
out vec3 fragNorm;
out vec3 fragColour;
uniform mat4 rotation;
uniform vec4 qAxis;
uniform vec3 movement3D;
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
    float f = 4.0/(position.z + 2.0);
    position2.x = f*position.x;
    position2.y = f*position.y;
    position2.z = position.z;
    return position2;
}

vec3 zCentredProject(vec3 p0, float zCameraLocation,
                     float nearPlaneWidth, float nearPlaneHeight,
                     float zNear, float zFar) {
    float z = p0.z - zCameraLocation;
    zNear = zNear - zCameraLocation;
    zFar = zFar - zCameraLocation;
    // linear
    // -(zFar + zNear)/(zFar - zNear) + 2.0*z/(zFar - zNear)
    return vec3(
        zNear*p0.x/(z*nearPlaneWidth/2.0),
        zNear*p0.y/(z*nearPlaneHeight/2.0),
        (zFar + zNear)/(zFar - zNear) - (2.0*zFar*zNear/z)/(zFar - zNear)
    );
}


vec4 zProject(vec3 p0, float zCameraLocation,
              float nearPlaneWidth, float nearPlaneHeight,
              float zNear, float zFar) {
    float z = p0.z - zCameraLocation;
    zNear = zNear - zCameraLocation;
    zFar = zFar - zCameraLocation;
    return vec4(
        zNear*p0.x/(nearPlaneWidth/2.0),
        zNear*p0.y/(nearPlaneHeight/2.0),
        (zFar + zNear)/(zFar - zNear)*z - (2.0*zFar*zNear)/(zFar - zNear),
        z
    );
}


float getDiffuseLighting(vec3 orientation, vec3 incomingLightRay)  {
    vec3 normalizedLightRay = normalize(incomingLightRay);
    vec3 normalizedOrientation = normalize(orientation);
    float lightVal = max(0.75*dot(normalizedOrientation, normalizedLightRay), 0.0);
    return lightVal;
}

vec3 translate(vec3 position, vec3 movement3D) {
    vec3 positionTranslated = vec3(
        position.x - movement3D.x*0.02,
        position.y - movement3D.z*0.02,
        position.z - movement3D.y*0.02
    );
    return positionTranslated;
}

void main () {
    vec3 positionScaled = position*scale;
    vec3 positionRotated = rotate(positionScaled, qAxis);
    vec3 positionTranslated = translate(positionRotated, movement3D);
    vec3 normalRotated = rotate(normal, qAxis);
    vec3 cameraPosition = vec3(0.0, 0.0, -2.0);
    vec3 cameraToPosition = cameraPosition - positionTranslated;
    // vec3 position3 = perspectiveProject(positionTranslated);
    fragPosition = positionTranslated;
    fragNorm = normalRotated;
    fragTexture = texture;
    fragColour = inColour;

    // vec3 lightVec = vec3(1.0, 0.0, 1.0);
    // vec3 reflectionVec = reflect(lightVec, normalRotated);
    // float specular = pow(max(0.0, dot(reflectionVec, normalize(cameraToPosition))), 4);
    // outColour = lightingColour(inColour, normalRotated, lightVec);

    // Doing all the projection calculations in the vertex shader
    // and passing a 1 to the w component leads to the textures being
    // unproperly wrapped (they aren't transformed to the actual perspective).
    /*vec3 positionProjected = zCentredProject(
        positionTranslated, cameraPosition.z, 1.0, 1.0, -1.0, 4.0
    );
    gl_Position = vec4(positionProjected, 1.0);*/

    gl_Position = zProject(positionTranslated, 
                           cameraPosition.z, 1.0, 1.0, -1.5, 4.0);
}
