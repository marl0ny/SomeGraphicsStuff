let canvas = document.getElementById("sketch-canvas");
let gl = canvas.getContext("webgl");


const vertexShaderSource = `
attribute vec4 pos;
attribute vec4 color;
uniform mat4 modelView;
uniform mat4 rotation;
uniform mat4 proj;

void main() {
    gl_Position = rotation*pos;
}
`;


const fragmentShaderSource = `
precision mediump float;
uniform float u_time;
void main () {
    gl_FragColor = vec4(cos(u_time), sin(u_time), 1.0, 1.0); 
}
`;


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


let vertexShaderID = makeShader(gl.VERTEX_SHADER, vertexShaderSource);
let fragmentShaderID = makeShader(gl.FRAGMENT_SHADER, fragmentShaderSource);
let shaderProgram = makeProgram(vertexShaderID, fragmentShaderID);
gl.useProgram(shaderProgram);


let vertexBufferID = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, vertexBufferID);
let vertices = [ 0.0, 0.5, 0.0, 1.0, 0.0, 0.0, 1.0, 
                -0.5, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0,
                 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0];
vertices = new Float32Array(vertices)
gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
// The signature of gl.bufferData is different than glBUfferData
// in OpenGL

let elementBufferID = gl.createBuffer();
gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, elementBufferID);
let elements = [0, 1, 2];
gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(elements), gl.STATIC_DRAW);

let posAttrib = gl.getAttribLocation(shaderProgram, 'pos');
gl.enableVertexAttribArray(posAttrib);
gl.vertexAttribPointer(posAttrib, 3, gl.FLOAT, false, 7*4, 0);

let rotationID = gl.getUniformLocation(shaderProgram, 'rotation');
let u_time = gl.getUniformLocation(shaderProgram, 'u_time');

gl.clearColor(0.0, 0.0, 0.0, 1.0);
gl.clearDepth(1.0); // clear everything
gl.enable(gl.DEPTH_TEST);
gl.depthFunc(gl.LEQUAL);
gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT); // clear the canvas
gl.drawArrays(gl.TRIANGLE_STRIP, 0, 3);


let theta = 0.015;
function animate() {
    theta += 0.01;
    let c = Math.cos(theta);
    let s = Math.sin(theta);
    let rotation = [
        c, 0.0,   s, 0.0, 
      0.0, 1.0, 0.0, 0.0, 
       -s, 0.0,   c, 0.0,
      0.0, 0.0, 0.0, 1.0
    ];
    gl.uniformMatrix4fv(rotationID, 0, rotation);
    gl.uniform1f(u_time, theta);
    // gl.bufferSubData(gl.ARRAY_BUFFER, 0, vertices);
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    // gl.drawArrays(gl.TRIANGLE_STRIP, 0, 3);
    // gl.drawElements()
    gl.drawElements(gl.TRIANGLES, 3, gl.UNSIGNED_SHORT, 0);
    requestAnimationFrame(animate);
}


animate();
