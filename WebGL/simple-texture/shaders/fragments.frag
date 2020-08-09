precision mediump float;
varying lowp vec4 color2;
varying lowp vec2 texture2;
varying lowp float lighting;
uniform sampler2D sampler;


void main () {
    highp vec4 texColor = texture2D(sampler, texture2);
    gl_FragColor = vec4(texColor.rgb*lighting, texColor.a);
    // gl_FragColor = color2;
}