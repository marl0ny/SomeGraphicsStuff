

function makeShader(gl, shaderType, shaderSource) {
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


function makeProgram(gl, ...shaderIDs) {
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


export {makeShader, makeProgram};
