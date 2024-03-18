#VERSION_NUMBER_PLACEHOLDER

#if (__VERSION__ >= 330) || (defined(GL_ES) && __VERSION__ >= 300)
#define texture2D texture
#else
#define texture texture2D
#endif

#if __VERSION__ <= 120
attribute vec4 inputData;
attribute vec4 COLOUR;
#else
in vec4 inputData;
out vec4 COLOUR;
#endif

uniform float viewScale;
uniform float vecScale;

uniform sampler2D vecTex;
uniform sampler2D colTex;


void main() {
    COLOUR = texture2D(colTex, inputData.xy);
    // Convert the texture coordinates of the inputData
    //  attribute to 3D texture coordinates
    vec3 r1 = 2.0*(vec3(inputData.xy, 0.0) - vec3(0.5, 0.5, 0.0));
    // Get raw texture data
    vec3 v1 = texture2D(vecTex, inputData.xy).xyz;
    // If vector is greater that a certain length, normalize it
    vec3 v2 = (length(v1) > 0.2)? 0.2*normalize(v1): v1; 
    gl_Position = vec4(viewScale*(r1 + v2*inputData.w*vecScale), 1.0);

}