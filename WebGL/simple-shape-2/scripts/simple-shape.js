import {vertexShaderSource, fragmentShaderSource} from "./shader-sources.js"
import {
    quaternionMultiply, 
    rotationAxisToQuaternion, 
    crossProduct} from "./helper-functions.js"


let canvas = document.getElementById("sketch-canvas");
let gl = canvas.getContext("webgl");


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
let h = 0.5, b = 0.25, a = 0.5;
let vertices = [
    a  , -a ,   a, 1.0, 0.0, 1.0, 1.0, 1.0,
    a  , a  , a  , 1.0, 0.0, 1.0, 1.0, 1.0,
    -a , a  , a  , 1.0, 0.0, 1.0, 1.0, 1.0,
    -a , -a , a  , 1.0, 0.0, 1.0, 1.0, 1.0,
     a , -a , -a , 1.0, 0.0, 1.0, 1.0, 1.0,
     a , a  , -a , 1.0, 0.0, 1.0, 1.0, 1.0,
    -a , a  , -a , 1.0, 0.0, 1.0, 1.0, 1.0,
    -a , -a , -a , 1.0, 0.0, 1.0, 1.0, 1.0
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
    0, 4, 5,
    1, 0, 5,
    0, 3, 7,
    0, 7, 4,
    3, 2, 6,
    3, 6, 7,
    2, 1, 5,
    6, 2, 5,
    5, 6, 5,
    0, 1, 3,
    1, 2, 3,
    6, 5, 4,
    7, 6, 4
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
                [0.0, 0.0, 1.0]
            );
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


document.addEventListener("mousemove", ev => handleMouseInput(ev, true));
document.addEventListener("mouseup", ev => mouseInput.isActive = false);
document.addEventListener("mousedown", ev => handleMouseInput(ev, false));
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
    gl.drawElements(gl.TRIANGLES, 39, gl.UNSIGNED_SHORT, 0);
    requestAnimationFrame(animate);
}


animate();
