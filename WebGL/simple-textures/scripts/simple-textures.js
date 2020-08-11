import {makeShader, makeProgram} from "./gl-wrappers.js"
import {vertexShaderSource, fragmentShaderSource} from "./shader-sources.js"
import {
    quaternionMultiply, 
    rotationAxisToQuaternion, quaternionRotate, quaternionConjugate,
    crossProduct, dot} from "./math-functions.js"
import {makeCylinder, makeSphere} from "./make-objects.js"
import {MouseInput} from "./mouse-handler.js"


let canvas = document.getElementById("sketch-canvas");
let gl = canvas.getContext("webgl");
let image = new Image();
image.src = "../../Textures/world.bmp";
new Promise(e => setTimeout(main, 500));


function main() {

    let vertexShaderID = makeShader(gl, gl.VERTEX_SHADER, vertexShaderSource);
    let fragmentShaderID = makeShader(gl, gl.FRAGMENT_SHADER, fragmentShaderSource);
    let shaderProgram = makeProgram(gl, vertexShaderID, fragmentShaderID);
    gl.useProgram(shaderProgram);

    let textureID = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, textureID);
    gl.texImage2D(
        gl.TEXTURE_2D, 0, gl.RGBA, 
        1, 1, 0, 
        gl.RGBA, gl.UNSIGNED_BYTE, 
        new Uint8Array([255, 0, 255, 255])
    );
    gl.texImage2D(
        gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image
    );
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);

    let vertexBufferID = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBufferID);
    let cylinder1 = makeCylinder(0.01, 2.0*Math.sqrt(0.5), 25);
    let cylinder2 = makeCylinder(0.5, 0.25*Math.sqrt(0.5), 4);
    let sphere = makeSphere(1.0, 150, 150);
    /* for (let i = 0; i < cylinder1.length; i++) {
        cylinder1[i] -= (i % 13 === 1)? 0.125: 0.0;
    }
    cylinder1.map(e => cylinder2.push(e));*/
    let vertices = new Float32Array(sphere);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

    let posAttrib = gl.getAttribLocation(shaderProgram, 'pos');
    gl.enableVertexAttribArray(posAttrib);
    gl.vertexAttribPointer(posAttrib, 4, gl.FLOAT, false, 13*4, 0);

    let texAttrib = gl.getAttribLocation(shaderProgram, 'texture');
    gl.enableVertexAttribArray(texAttrib);
    gl.vertexAttribPointer(texAttrib, 2, gl.FLOAT, false, 13*4, 4*4);

    let normAttrib = gl.getAttribLocation(shaderProgram, 'normal');
    gl.enableVertexAttribArray(normAttrib);
    gl.vertexAttribPointer(normAttrib, 3, gl.FLOAT, false, 13*4, 4*6);

    /*let colAttrib = gl.getAttribLocation(shaderProgram, 'color');
    gl.enableVertexAttribArray(colAttrib);
    gl.vertexAttribPointer(colAttrib, 4, gl.FLOAT, false, 13*4, 4*9);*/

    let scale = 1.0;
    let scaleID = gl.getUniformLocation(shaderProgram, 'scale');
    let rotationQuaternion = gl.getUniformLocation(
        shaderProgram, 'rotationQuaternion');
    let samplerID = gl.getUniformLocation(shaderProgram, 'sampler');
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, textureID);
    gl.uniform1i(samplerID, 0);

    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    let q = [1.0, 0.0, 0.0, 0.0];
    function mouseHandlerFunction (mouseHandler) {
        let deltaX = mouseHandler.getDeltaX();
        let deltaY = mouseHandler.getDeltaY();
        if (deltaX !== 0.0 || deltaY !== 0.0) {
            let angle = 10*Math.sqrt(
                deltaX**2 + deltaY**2)/canvas.width;
            let axis = crossProduct(
                [-deltaX, deltaY, 0.0], [0.0, 0.0, 1.0]);
            q = quaternionMultiply(q, rotationAxisToQuaternion(angle, axis));
        }
    }

    let zOrientation = [0.0, 1.0, 0.0];
    function mouseHandlerFunction2 (mouseHandler) {
        let sensitivity = 7.0;
        let deltaX = mouseHandler.getDeltaX();
        let deltaY = mouseHandler.getDeltaY();
        let qAxis = [1.0, 0.0, 0.0, 0.0];
        if (deltaY != 0.0) {
            let axis = crossProduct([0.0, deltaY, 0.0], [0.0, 0.0, 1.0]);
            let angle = sensitivity*Math.abs(deltaY)/canvas.width;
            qAxis = rotationAxisToQuaternion(angle, axis);
            q = quaternionMultiply(q, qAxis);
        }
        zOrientation = quaternionRotate(qAxis, zOrientation);
        if (deltaX != 0.0) {
            // let axis = crossProduct([deltaX, 0.0, 0.0], [0.0, 0.0, 1.0]);
            // let angle = -sensitivity*dot(axis, zOrientation)/canvas.width;
            let angle = sensitivity*deltaX/canvas.width;
            let qAxis = rotationAxisToQuaternion(angle, zOrientation);
            q = quaternionMultiply(q, qAxis);
        }
    }

    let mouseHandler = new MouseInput();
    mouseHandler.setMouseMoveCallback(mouseHandlerFunction);
    document.addEventListener("mousemove", 
                              ev => mouseHandler.handleMouseInput(ev, true));
    document.addEventListener("mouseup", ev => mouseHandler.deactivate());
    document.addEventListener("mousedown", 
                              ev => mouseHandler.handleMouseInput(ev, false));
    document.addEventListener(
        "wheel", ev => scale *= (ev.deltaY > 0)? 0.9: 1.1);

    function animate() {
        gl.uniform1f(scaleID, scale);
        gl.uniform4fv(rotationQuaternion, 
            [q[1], q[2], q[3], q[0]], 0, 4);
        gl.clearColor(0.0, 0.0, 0.0, 1.0);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
        gl.drawArrays(gl.TRIANGLES, 0, vertices.length/13);
        // gl.drawArrays(gl.LINES, 0, vertices.length);
        requestAnimationFrame(animate);
    }

    animate();

}
