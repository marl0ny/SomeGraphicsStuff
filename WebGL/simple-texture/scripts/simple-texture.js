import {vertexShaderSource, fragmentShaderSource} from "./shader-sources.js"
import {
    quaternionMultiply, 
    rotationAxisToQuaternion, 
    crossProduct,
    makeCylinder} from "./helper-functions.js"


let image = new Image();
image.src = "./texture-2.jpg";
setTimeout(main, 500);


function main() {

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
    /*
    let vertices = [
        0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0,
        0.5,-0.5, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0,
       -0.5,-0.5, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
       -0.5,-0.5, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
       -0.5, 0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0,
        0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0,
        ];
    vertices = new Float32Array(vertices)*/
    let vertices = makeCylinder(1.0, Math.sqrt(0.5), 25);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
    // The signature of gl.bufferData is different than glBufferData
    // in OpenGL

    let posAttrib = gl.getAttribLocation(shaderProgram, 'pos');
    gl.enableVertexAttribArray(posAttrib);
    gl.vertexAttribPointer(posAttrib, 4, gl.FLOAT, false, 13*4, 0);

    let texAttrib = gl.getAttribLocation(shaderProgram, 'texture');
    gl.enableVertexAttribArray(texAttrib);
    gl.vertexAttribPointer(texAttrib, 2, gl.FLOAT, false, 13*4, 4*4);

    let normAttrib = gl.getAttribLocation(shaderProgram, 'normal');
    gl.enableVertexAttribArray(normAttrib);
    gl.vertexAttribPointer(normAttrib, 3, gl.FLOAT, false, 13*4, 4*6);

    /* let colAttrib = gl.getAttribLocation(shaderProgram, 'color');
    gl.enableVertexAttribArray(colAttrib);
    gl.vertexAttribPointer(colAttrib, 4, gl.FLOAT, false, 13*4, 4*9);*/


    let rotationID = gl.getUniformLocation(shaderProgram, 'rotation');
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
        gl.drawArrays(gl.TRIANGLES, 0, vertices.length/(13));
        // gl.drawArrays(gl.LINES, 0, vertices.length);
        requestAnimationFrame(animate);
    }


    animate();

}