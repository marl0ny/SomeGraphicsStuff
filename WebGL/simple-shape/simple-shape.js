let canvas = document.getElementById("sketch-canvas");
let gl = canvas.getContext("webgl");


const vertexShaderSource = `
precision mediump float;
attribute vec4 pos;
attribute vec4 color;
varying lowp vec4 color2;
uniform mat4 modelView;
uniform mat4 rotation;
uniform mat4 proj;
uniform float scale;
uniform vec4 rotationQuaternion;


vec4 quaternionMultiply(vec4 q1, vec4 q2) {
    vec4 q3;
    q3.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
    q3.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y; 
    q3.y = q1.w*q2.y + q1.y*q2.w + q1.z*q2.x - q1.x*q2.z; 
    q3.z = q1.w*q2.z + q1.z*q2.w + q1.x*q2.y - q1.y*q2.x;
    return q3; 
}


vec4 rotate(vec4 x, vec4 r) {
    vec4 xr = quaternionMultiply(x, r);
    r.x = -r.x;
    r.y = -r.y;
    r.z = -r.z;
    vec4 x2 = quaternionMultiply(r, xr);
    x2.w = 1.0;
    return x2; 
}


vec4 project(vec4 x) {
    vec4 y;
    y[0] = x[0]*5.0/(x[2] + 5.0);
    y[1] = x[1]*5.0/(x[2] + 5.0);
    y[2] = x[2];
    y[3] = 1.0;
    return y;
}


void main() {
    color2 = color;
    gl_Position = project(rotate(scale*pos, rotationQuaternion));
}
`;


const fragmentShaderSource = `
precision mediump float;
varying lowp vec4 color2;
void main () {
    gl_FragColor = color2;
}
`;


function quaternionMultiply(q1, q2) {
    let q3 = [0.0, 0.0, 0.0, 0.0];
    q3[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    q3[1] = q1[0]*q2[1] + q1[1]*q2[0] + q1[2]*q2[3] - q1[3]*q2[2]; 
    q3[2] = q1[0]*q2[2] + q1[2]*q2[0] + q1[3]*q2[1] - q1[1]*q2[3]; 
    q3[3] = q1[0]*q2[3] + q1[3]*q2[0] + q1[1]*q2[2] - q1[2]*q2[1];
    return q3;
}


function rotationAxisToQuaternion(angle, axis) {
    let norm = Math.sqrt(axis[0]**2 + axis[1]**2 + axis[2]**2);
    for (let i = 0; i < 3; i++) {
        axis[i] = axis[i]/norm;
    }
    let c = Math.cos(angle/2.0);
    let s = Math.sin(angle/2.0);
    return [c, s*axis[0], s*axis[1], s*axis[2]];
}


function crossProduct(v, w) {
    return [
        v[1]*w[2] - v[2]*w[1],
        v[2]*w[0] - v[0]*w[2],
        v[0]*w[1] - v[1]*w[0]
    ]
}


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
let h = 0.5, b = 0.25;
let vertices = [
    h  , 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0,
    0.0, b  , b  , 1.0, 0.0, 1.0, 0.0, 1.0,
    0.0, b  , -b , 1.0, 1.0, 0.0, 0.0, 1.0,
    0.0, -b , -b , 1.0, 0.0, 0.0, 1.0, 1.0,
    0.0, -b , b  , 1.0, 0.0, 1.0, 0.0, 1.0,
     -h, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
    ];
vertices = new Float32Array(vertices)
gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
// The signature of gl.bufferData is different than glBufferData
// in OpenGL

let posAttrib = gl.getAttribLocation(shaderProgram, 'pos');
gl.enableVertexAttribArray(posAttrib);
gl.vertexAttribPointer(posAttrib, 4, gl.FLOAT, false, 8*4, 0);

let colAttrib = gl.getAttribLocation(shaderProgram, 'color');
gl.enableVertexAttribArray(colAttrib);
gl.vertexAttribPointer(colAttrib, 4, gl.FLOAT, false, 8*4, 4*4);

let elementBufferID = gl.createBuffer();
gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, elementBufferID);
let elements = [
    1, 0, 4,
    0, 3, 4,
    0, 2, 3,
    0, 1, 2,
    1, 4, 5,
    4, 3, 5,
    3, 2, 5,
    1, 5, 2
];
gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(elements), gl.STATIC_DRAW);

let rotationID = gl.getUniformLocation(shaderProgram, 'rotation');
let scale = 1.0;
let scaleID = gl.getUniformLocation(shaderProgram, 'scale');
let rotationQuaternion = gl.getUniformLocation(
    shaderProgram, 'rotationQuaternion');

gl.enable(gl.DEPTH_TEST);
gl.depthFunc(gl.LEQUAL);


let mouseInput = {
    isActive: false,
    prev: {x: 0.0, y: 0.0},
    curr: {x: 0.0, y: 0.0},
    dxy: {x: 0.0, y: 0.0}
}


let q = [1.0, 0.0, 0.0, 0.0];
function handleMouseInput(event, mouseMove=true) {
    let x = event.clientX;
    let y = event.clientY;
    if (mouseMove && mouseInput.isActive) {
        mouseInput.dxy.x = x - mouseInput.prev.x
        mouseInput.dxy.y = y - mouseInput.prev.y
        mouseInput.prev.x = mouseInput.curr.x;
        mouseInput.prev.y = mouseInput.curr.y;
        mouseInput.curr.x = x;
        mouseInput.curr.y = y;
        if (mouseInput.dxy.x !== 0.0 || mouseInput.dxy.y !== 0.0) {
            let angle = 10*Math.sqrt(
                mouseInput.dxy.x**2 + mouseInput.dxy.y**2)/canvas.width;
            let axis = crossProduct(
                [-mouseInput.dxy.x, mouseInput.dxy.y, 0.0],
                [0.0, 0.0, 1.0]);
            q = quaternionMultiply(
                q, rotationAxisToQuaternion(angle, axis));
        }
    } else if (!mouseInput.isActive) {
        mouseInput.curr.x = x;
        mouseInput.curr.y = y;
        mouseInput.prev.x = x;
        mouseInput.prev.y = y;
        if (!mouseMove) mouseInput.isActive = true;
    }
}


document.addEventListener("mousemove", ev => handleMouseInput(ev));
document.addEventListener("mouseup", ev => mouseInput.isActive = false);
document.addEventListener("mousedown", ev => handleMouseInput(ev, mouseMove=false));
document.addEventListener(
    "wheel", ev => scale *= (ev.deltaY > 0)? 0.9: 1.1);


let theta = 0.015;
function animate() {
    theta += 0.01;
    gl.uniform1f(scaleID, scale);
    gl.uniform4fv(rotationQuaternion, 
        [q[1], q[2], q[3], q[0]], 0, 4);
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.drawElements(gl.TRIANGLES, 24, gl.UNSIGNED_SHORT, 0);
    requestAnimationFrame(animate);
}


animate();
