precision mediump float;
varying lowp vec4 color2;
uniform sampler2D sampler;
varying highp vec2 textureCoord;


void main () {
    // gl_FragColor = texture2D(sampler, textureCoord);
    gl_FragColor = color2;
}
