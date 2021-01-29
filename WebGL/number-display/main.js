let vertShader = makeShader(gl.VERTEX_SHADER, vertexSrc);
let viewShader = makeShader(gl.FRAGMENT_SHADER, viewSrc);
let viewProgram = makeProgram(vertShader, viewShader);

let draw = () => gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);

let width = canvas.width;
let height = canvas.height;


let image = new Image();
image.src = "font.bmp"

setTimeout(() => main(), 1000);

function main() {
    
    let tex = makeImageTexture(image);

    let frame = new Frame(width, height, 0);
    frame.useProgram(viewProgram);
    frame.bind();
    frame.setIntUniforms({"tex1": 0});
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, tex);
    draw();
    unbind();


    let mouseXY = [0.0, 0.0];
    let t = 0
    let ct = ''
    function animate() {
        t += 0.00001
        ct = '' + t
        console.log(ct);
        if (ct.length <= 5) ct += '00'
        frame.useProgram(viewProgram);
        frame.bind();
        frame.setIntUniforms({"tex1": 0});
        frame.setFloatUniforms({"mouseX": mouseXY[0], "mouseY": mouseXY[1]});
        let f = new FontManager(fontJsonData);
        let cData = f.getCharacterData(ct[0]);
        frame.setVec4Uniforms({"char1": [cData.x, cData.y,
					 cData.x + cData.width, cData.y + cData.height
					]});
        f = new FontManager(fontJsonData);
        cData = f.getCharacterData(ct[1]);
        frame.setVec4Uniforms({"char2": [cData.x, cData.y,
					 cData.x + cData.width, cData.y + cData.height
					]});
        f = new FontManager(fontJsonData);
        cData = f.getCharacterData(ct[2]);
        frame.setVec4Uniforms({"char3": [cData.x, cData.y,
					 cData.x + cData.width, cData.y + cData.height
					]});
        f = new FontManager(fontJsonData);
        cData = f.getCharacterData(ct[3]);
        frame.setVec4Uniforms({"char4": [cData.x, cData.y,
					 cData.x + cData.width, cData.y + cData.height
					]});
        f = new FontManager(fontJsonData);
        cData = f.getCharacterData(ct[4]);
        frame.setVec4Uniforms({"char5": [cData.x, cData.y,
					 cData.x + cData.width, cData.y + cData.height
					]});
        gl.bindTexture(gl.TEXTURE_2D, tex);
        draw();
        unbind();
        requestAnimationFrame(animate);
        t = t % 10.0
    }
    canvas.addEventListener("mousemove", ev => {
        mouseXY[0] = (ev.clientX - canvas.offsetLeft)/parseInt(canvas.style.width);
        mouseXY[1] = (ev.clientY - canvas.offsetTop)/parseInt(canvas.style.height);
        console.log(mouseXY[0], mouseXY[1]);
    });

    animate();
}
