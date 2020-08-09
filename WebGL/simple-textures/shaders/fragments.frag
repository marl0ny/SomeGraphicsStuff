precision mediump float;
varying mediump vec2 vTexture;
varying mediump vec4 vColor;
varying mediump vec4 vPos;
varying mediump vec4 vNorm;
uniform sampler2D sampler;


float diffuseLighting(vec4 color, vec3 normal, vec3 lightRay) {
    vec3 nLightRay = normalize(lightRay);
    vec3 nNormal = normalize(normal);
    return max(dot(-nLightRay, nNormal), 0.0);
}


float specularLighting(vec3 reflectDir, vec3 lightRay) {
    vec3 nLightRay = normalize(lightRay);
    vec3 nReflectDir = normalize(reflectDir);
    float s = dot(nReflectDir, nLightRay);
    return max(s*s*s*s, 0.0);
}


void main () {
    vec3 cameraSource = vec3(-2.0, 0.0, 0.0);
    vec3 lightSource = vec3(-10.0, 0.0, -10.0);
    vec3 lightSourceToPos = vec3(vPos) - lightSource;
    vec3 cameraToPos = vec3(vPos) - cameraSource;
    highp vec4 texColor = texture2D(sampler, vTexture);
    float diffuse = diffuseLighting(texColor, vec3(vNorm), lightSourceToPos);
    float specular = specularLighting(
        reflect(cameraToPos, normalize(vec3(vNorm))), lightSourceToPos);
    float lighting =  diffuse + specular + 0.15;
    gl_FragColor = vec4(texColor.rgb*lighting, texColor.a);
    // gl_FragColor = color2;
}