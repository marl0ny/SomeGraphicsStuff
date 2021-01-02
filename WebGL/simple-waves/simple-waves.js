let canvas = document.getElementById("sketch-canvas");
let gl = canvas.getContext("webgl");
gl.getExtension('OES_texture_float');
gl.getExtension('OES_texture_float_linear');


const vertexShaderSource = `
attribute vec3 pos;
varying highp vec2 fragTexCoord;

void main() {
    gl_Position = vec4(pos.xyz, 1.0);
    fragTexCoord = vec2(0.5, 0.5) + pos.xy/2.0;
}
`;


const wavePropagationFragmentSource = `
precision highp float;
varying highp vec2 fragTexCoord;
uniform float dx;
uniform float dy;
uniform sampler2D tex1;
uniform sampler2D tex2;

void main () {
    if (fragTexCoord.x > dx && fragTexCoord.x < 1.0-dx &&
        fragTexCoord.y > dy && fragTexCoord.y < 1.0-dy) {
        vec4 l = texture2D(tex2, fragTexCoord + vec2(dx, 0.0));
        vec4 r = texture2D(tex2, fragTexCoord + vec2(-dx, 0.0));
        vec4 u = texture2D(tex2, fragTexCoord + vec2(0.0, dy));
        vec4 d = texture2D(tex2, fragTexCoord + vec2(0.0, -dy));
        vec4 c = texture2D(tex1, fragTexCoord);
        gl_FragColor = (u + d + l + r)/2.0 - c;
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); 
    }
}
`

const waveCreationFragmentSource = `
precision highp float;
varying highp vec2 fragTexCoord;
uniform float dx;
uniform float dy;
uniform float bx;
uniform float by;

void main () {
    if (fragTexCoord.x > dx && fragTexCoord.x < 1.0-dx &&
        fragTexCoord.y > dy && fragTexCoord.y < 1.0-dy) {
        float x = fragTexCoord.x;
        float y = fragTexCoord.y;
        float sx = 1.4142135623730951/(1.0/(dx*10.0));
        float sy = 1.4142135623730951/(1.0/(dy*10.0));
        float u = ((x - bx)/sx), v = ((y - by)/sy);
        float val = 5.0*exp(- u*u - v*v);
        gl_FragColor = vec4(val, 0.0, 0.0, 1.0); 
    gl_FragColor = vec4(val, 0.0, 0.0, 1.0); 
        gl_FragColor = vec4(val, 0.0, 0.0, 1.0); 
    gl_FragColor = vec4(val, 0.0, 0.0, 1.0); 
        gl_FragColor = vec4(val, 0.0, 0.0, 1.0); 
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); 
    }
}
`

const viewFrameFragmentSource = `
precision highp float;
varying highp vec2 fragTexCoord;
uniform sampler2D tex1;

void main () {
    vec4 col = texture2D(tex1, fragTexCoord);
    col.r += 25.0/255.0;
    gl_FragColor = vec4(col.r/4.0, col.r/2.0, col.r, 1.0); 
}
`

const copyOverFragmentSource = `
precision highp float;
varying highp vec2 fragTexCoord;
uniform sampler2D tex1;
uniform sampler2D tex2;

void main () {
    vec4 col1 = texture2D(tex1, fragTexCoord);
    vec4 col2 = texture2D(tex2, fragTexCoord);
    gl_FragColor = vec4(col1.r + col2.r, 0.0, 0.0, 1.0); 
}
`

function makeShader(shaderType, shaderSource) {
    let shaderID = gl.createShader(shaderType);
    if (shaderID === 0) {
        alert("Unable to create shader.");
    }
    gl.shaderSource(shaderID, shaderSource);
    gl.compileShader(shaderID);
    if (!gl.getShaderParameter(shaderID, gl.COMPILE_STATUS)) {
        let msg = gl.getShaderInfoLog(shaderID);
        alert(`Unable to compile shader.\n${msg}`);
        gl.deleteShader(shaderID);
    }
    return shaderID;
}

function makeProgram(...shaderIDs) {
    let shaderProgram = gl.createProgram();
    for (let shaderID of shaderIDs) {
        gl.attachShader(shaderProgram, shaderID);
    }
    gl.linkProgram(shaderProgram);
    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        alert(
            'Unable to initialize the shader program: '
            + gl.getProgramInfoLog(shaderProgram));
    }
    return shaderProgram;
}

function unbind() {
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
}

function makeTexture(buf, w, h) {
    let texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(
        gl.TEXTURE_2D, 0, gl.RGBA, 
        w, h, 0, 
        gl.RGBA, gl.FLOAT, 
        buf
    );
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.generateMipmap(gl.TEXTURE_2D);
    return texture;
}

class Frame {
    constructor(w, h, frameNumber) {
        unbind();
        this.shaderProgram = null;
        this.uniforms = {};
        this.frameNumber = frameNumber;
        this.frameTexture = null;
        if (this.frameNumber !== 0) {
            gl.activeTexture(gl.TEXTURE0 + frameNumber);
            this.frameTexture = makeTexture(null, w, h);
            gl.bindTexture(gl.TEXTURE_2D, this.frameTexture);
        }
        this.vbo = gl.createBuffer();
        this.ebo = gl.createBuffer();
        this.fbo = gl.createFramebuffer();
        if (this.frameNumber !== 0) {
            gl.bindFramebuffer(gl.FRAMEBUFFER, this.fbo);
            gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, 
                                    gl.TEXTURE_2D, this.frameTexture, 0);
        }
        unbind();
    }
    setFloatUniforms(uniforms) {
        for (let field of Object.keys(uniforms)) {
            this.uniforms[field] = gl.getUniformLocation(this.shaderProgram, field);
            gl.uniform1f(this.uniforms[field], uniforms[field]);
        }
    }
    setIntUniforms(uniforms) {
        for (let field of Object.keys(uniforms)) {
            this.uniforms[field] = gl.getUniformLocation(this.shaderProgram, field);
            gl.uniform1i(this.uniforms[field], uniforms[field]);
        }
    }
    useProgram(shaderProgram) {
        this.shaderProgram = shaderProgram;
        gl.useProgram(shaderProgram);
    }
    bind() {
        if (this.frameNumber !== 0) {
            gl.bindFramebuffer(gl.FRAMEBUFFER, this.fbo);
        }
        let shaderProgram = this.shaderProgram;
        let vertices = new Float32Array([1.0, 1.0, 0.0, 1.0, -1.0, 0.0, 
            -1.0, -1.0, 0.0, -1.0, 1.0, 0.0]);
        let elements = new Uint16Array([0, 2, 3, 0, 1, 2]);
        gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo);
        gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.ebo);
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, elements, gl.STATIC_DRAW);
        let pos = gl.getAttribLocation(shaderProgram, 'pos');
        gl.enableVertexAttribArray(pos);
        gl.vertexAttribPointer(pos, 3, gl.FLOAT, false, 3*4, 0);
    }
}


let vShader = makeShader(gl.VERTEX_SHADER, vertexShaderSource);
let timeStepShader = makeShader(gl.FRAGMENT_SHADER, wavePropagationFragmentSource);
let timeStepProgram = makeProgram(vShader, timeStepShader);
let waveCreationShader = makeShader(gl.FRAGMENT_SHADER, waveCreationFragmentSource);
let waveCreationProgram = makeProgram(vShader, waveCreationShader);
let displayShader = makeShader(gl.FRAGMENT_SHADER, viewFrameFragmentSource);
let displayProgram = makeProgram(vShader, displayShader);
let copyToShader = makeShader(gl.FRAGMENT_SHADER, copyOverFragmentSource);
let copyToProgram = makeProgram(vShader, copyToShader);


let w = canvas.width, h = canvas.height;

new Promise(() => setTimeout(main, 500));
function main() {

    let viewFrame = new Frame(w, h, 0);
    let swapFrames = [1, 2, 3].map(i => new Frame(w, h, i));
    let storeFrame = new Frame(w, h, 4);

    let bx = 0; let by = 0;
    let draw = () => gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);

    function createNewWave() {
        gl.activeTexture(gl.TEXTURE0);
        swapFrames[2].useProgram(waveCreationProgram);
        swapFrames[2].bind();
        gl.bindTexture(gl.TEXTURE_2D, makeTexture(null, w, h));
        swapFrames[2].setFloatUniforms({dx: 1.0/w, dy: 1.0/h,
                                            bx: bx/canvas.width, 
                                            by: 1.0 - by/canvas.height});
        draw();
        unbind();
        gl.activeTexture(gl.TEXTURE0 + swapFrames[2].frameNumber);
        gl.bindTexture(gl.TEXTURE_2D, swapFrames[2].frameTexture);
        storeFrame.useProgram(copyToProgram);
        storeFrame.bind();
        storeFrame.setIntUniforms({tex1: swapFrames[2].frameNumber, tex2: 5});
        draw();
        unbind();
    }

    function copyNewWaveToFrames() {
        for (let k = 0; k < swapFrames.length - 1; k++) {
            swapFrames[2].useProgram(copyToProgram);
            swapFrames[2].bind();
            swapFrames[2].setIntUniforms({tex1: swapFrames[k].frameNumber, tex2: 5});
            draw();
            unbind();
            swapFrames[k].useProgram(copyToProgram);
            swapFrames[k].bind();
            swapFrames[k].setIntUniforms({tex1: storeFrame.frameNumber, 
                                            tex2: swapFrames[2].frameNumber});
            draw();
            unbind();
        }
    }

    function timeStepWave() {
        swapFrames[2].useProgram(timeStepProgram);
        swapFrames[2].bind();
        swapFrames[2].setFloatUniforms({dx: 1.0/w, dy: 1.0/h});
        swapFrames[2].setIntUniforms({tex1: swapFrames[0].frameNumber, 
                                        tex2: swapFrames[1].frameNumber});
        draw();
        unbind();
    }

    function display() {
        gl.activeTexture(gl.TEXTURE0);
        viewFrame.useProgram(displayProgram);
        viewFrame.bind();
        viewFrame.setIntUniforms({tex1: swapFrames[2].frameNumber});
        draw();
        unbind();
    }

    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    let justReleased = false;
    function animate() {
        if (justReleased) {
            createNewWave();
            copyNewWaveToFrames();
            justReleased = false;
        }
        timeStepWave();
        display();
        swapFrames = [swapFrames[1], swapFrames[2], swapFrames[0]];
        requestAnimationFrame(animate);
    }

    let mouseReleaseWave = function(ev) {
        justReleased = true;
        bx = Math.floor((ev.clientX - canvas.offsetLeft));
        by = Math.floor((ev.clientY - canvas.offsetTop));
    }

    let touchReleaseWave = function(ev) {
        let touches = ev.changedTouches;
        if (touches.length >= 1) {
            bx = Math.floor((touches[0].pageX - canvas.offsetLeft));
            by = Math.floor((touches[0].pageY - canvas.offsetTop));
        }
    }

    document.addEventListener("touchstart", ev => touchReleaseWave(ev));
    document.addEventListener("touchmove", ev => touchReleaseWave(ev));   
    document.addEventListener("touchevent", ev => touchReleaseWave(ev));   
    document.addEventListener("mouseup", ev => mouseReleaseWave(ev));

    animate();
}
