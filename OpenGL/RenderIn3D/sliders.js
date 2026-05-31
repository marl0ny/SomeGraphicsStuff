const ENUM_CODES = {
    LINK: 0,
    T: 1,
    BRIGHTNESS: 2,
    SIMULATION_DIMENSIONS3_D: 3,
    DATA_TEXEL_DIMENSIONS3_D: 4,
    PRESET_FUNCTIONS_DROPDOWN: 5,
    USER_TEXT_ENTRY: 6,
    LATEX_LABEL: 7,
    VISUALIZATION_SELECT: 8,
    USE_PERSPECTIVE_PROJECTION: 9,
    VOLUME_RENDER_SECTION_START: 10,
    USE_LINEAR: 11,
    ALPHA_BRIGHTNESS: 12,
    COLOR_BRIGHTNESS: 13,
    VOLUME_TEXEL_DIMENSIONS3_D: 14,
    APPLY_BLUR: 15,
    BLUR_SIZE: 16,
    VOLUME_RENDER_SECTION_END: 17,
    PLANAR_SLICES_SECTION_START: 18,
    PLANAR_NORM_COORD_OFFSETS: 19,
    PLANAR_SLICES_SECTION_END: 20,
    ARROWS3_D_LINE_SECTION_START: 21,
    ARROW_DIMENSIONS: 22,
    USE_CONES: 23,
    ARROWS3_D_LINE_SECTION_END: 24,
    CANVAS_HOVER_DISPLAY: 25,
    DUMMY_VALUE: 26,
};

let gVecParams = {};
let gUserParams = {};
let gCheckboxXorLists = {};

function createScalarParameterSlider(
    controls, enumCode, sliderLabelName, type, spec) {
    let label = document.createElement("label");
    label.for = spec['id']
    // label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = `${sliderLabelName} = ${spec.value}`;
    label.id = `slider-label-${enumCode}`;
    controls.appendChild(label);
    let slider = document.createElement("input");
    slider.type = "range";
    slider.style ="width: 95%;"
    for (let k of Object.keys(spec))
        slider[k] = spec[k];
    slider.value = spec.value;
    slider.id = `slider-${enumCode}`;
    controls.appendChild(document.createElement("br"));
    controls.appendChild(slider);
    controls.appendChild(document.createElement("br"));
    slider.style.touchAction = 'none';
    if (isOnMobile())
        controls.appendChild(document.createElement("br"));
    slider.addEventListener("input", e => {
        let valueF = Number.parseFloat(e.target.value);
        let valueI = Number.parseInt(e.target.value);
        if (type === "float") {
            label.textContent = `${sliderLabelName} = ${valueF}`
            Module.set_float_param(enumCode, valueF);
        } else if (type === "int") {
            label.textContent = `${sliderLabelName} = ${valueI}`
            Module.set_int_param(enumCode, valueI);
        }
    });
};

function createCheckbox(controls, enumCode, name, value, xorListName='') {
    let label = document.createElement("label");
    // label.for = spec['id']
    // label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.innerHTML = `${name}`
    let checkbox = document.createElement("input");
    checkbox.type = "checkbox";
    checkbox.id = `checkbox-${enumCode}`;
    if (xorListName !== '') {
        if (!(xorListName in gCheckboxXorLists))
            gCheckboxXorLists[xorListName] = [checkbox.id];
        else
            gCheckboxXorLists[xorListName].push(checkbox.id);
    }
    // slider.style ="width: 95%;"
    // checkbox.value = value;
    checkbox.checked = value;
    // controls.appendChild(document.createElement("br"));
    controls.appendChild(checkbox);
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
    checkbox.addEventListener("input", e => {
        console.log(e.target.checked);
        Module.set_bool_param(enumCode, e.target.checked);
        if (e.target.checked === true && xorListName !== '') {
            for (let id_ of gCheckboxXorLists[xorListName]) {
                if (id_ !== checkbox.id) {
                    let enumCode2 = parseInt(id_.split('-')[1]);
                    Module.set_bool_param(enumCode2, false);
                    document.getElementById(id_).checked = false;
                }
            }
        }
    }
    );
}

function editScalarParameterSliderDisplay(enumCode, sliderLabelName, value) {
    let slider = document.getElementById(`slider-${enumCode}`);
    let label = document.getElementById(`slider-label-${enumCode}`);
    slider.value = value;
    label.textContent 
       = `${sliderLabelName} = ${value}`;
}

function editVectorParameterSliderDisplay(enumCode, sliderLabelName, index, value) {
    let slider = document.getElementById(`slider-${enumCode}-${index}`);
    let label = document.getElementById(`slider-label-${enumCode}`);
    slider.value = value;
    gVecParams[sliderLabelName][Number.parseInt(index)] = value;
    label.textContent 
        = `${sliderLabelName} = (${gVecParams[sliderLabelName]})`;
}

function createVectorParameterSliders(
    controls, enumCode, sliderLabelName, type, spec) {
    let label = document.createElement("label");
    // label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = `${sliderLabelName} = (${spec.value})`;
    label.id = `slider-label-${enumCode}`;
    gVecParams[sliderLabelName] = spec.value;
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
    for (let i = 0; i < spec.value.length; i++) {
        let slider = document.createElement("input");
        slider.type = "range";
        slider.style ="width: 95%;"
        for (let k of Object.keys(spec))
            slider[k] = spec[k][i];
        slider.value = spec.value[i];
        slider.id = `slider-${enumCode}-${i}`;
        controls.appendChild(slider);
        controls.appendChild(document.createElement("br"));
        slider.style.touchAction = 'none';
        slider.addEventListener("input", e => {
            let valueF = Number.parseFloat(e.target.value);
            let valueI = Number.parseInt(e.target.value);
            if (type === "Vec2" || 
                type === "Vec3" || type === "Vec4") {
                gVecParams[sliderLabelName][i] = valueF;
                label.textContent 
                    = `${sliderLabelName} = (${gVecParams[sliderLabelName]})`
                Module.set_vec_param(
                    enumCode, spec.value.length, i, valueF);
            } else if (type === "IVec2" || 
                        type === "IVec3" || type === "IVec4") {
                gVecParams[sliderLabelName][i] = valueI;
                label.textContent 
                    = `${sliderLabelName} = (${gVecParams[sliderLabelName]})`
                Module.set_ivec_param(
                    enumCode, spec.value.length, i, valueI);
            }
        });
    }
    if (isOnMobile())
        controls.appendChild(document.createElement("br"));
};

function createSelectionList(
    controls, enumCode, defaultVal, selectionBoxName, textOptions
) {
    let label = document.createElement("label");
    // label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = selectionBoxName;
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
    let selector = document.createElement("select");
    for (let i = 0; i < textOptions.length; i++) {
        let option = document.createElement("option");
        option.value = i;
        option.textContent = textOptions[i];
        selector.add(option);
    }
    selector.value = defaultVal;
    selector.addEventListener("change", e =>
        Module.selection_set(
            enumCode, Number.parseInt(e.target.value))
    );
    controls.appendChild(selector);
    controls.appendChild(document.createElement("br"));
}

function createUploadImage(
    controls, enumCode, name, w_code, h_code
) {
    let label = document.createElement("label");
    // label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = name;
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
    // im.id = `image-${enumCode}`;
    let uploadImage = document.createElement("input");
    uploadImage.type = "file";
    let im = document.createElement("img");
    im.hidden = true;
    let imCanvas = document.createElement("canvas");
    imCanvas.hidden = true;
    controls.appendChild(uploadImage);
    // controls.appendChild(document.createElement("br"));
    controls.appendChild(im);
    // controls.appendChild(document.createElement("br"));
    controls.appendChild(imCanvas);
    // controls.appendChild(document.createElement("br"));
    uploadImage.addEventListener(
        "change", () => {
            console.log("image uploaded");
            const reader = new FileReader();
            reader.onload = e => {
                im.src = e.target.result;
            }
            let loadImageToPotentialFunc = () => {
                let ctx = imCanvas.getContext("2d");
                let width = Module.get_int_param(ENUM_CODES[w_code]);
                let height = Module.get_int_param(ENUM_CODES[h_code]);
                let imW = im.width;
                let imH = im.height;
                imCanvas.setAttribute("width", width);
                imCanvas.setAttribute("height", height);
                let heightOffset = 0;
                let widthOffset = 0;
                if (imW/imH >= width/height) {
                    let ratio = (imW/imH)/(width/height);
                    widthOffset = parseInt(0.5*width*(1.0 - ratio));
                    ctx.drawImage(im, widthOffset, heightOffset,
                                width*(imW/imH)/(width/height), height);
                } else {
                    let ratio = (imH/imW)/(height/width);
                    heightOffset = parseInt(0.5*height*(1.0 - ratio));
                    ctx.drawImage(im, widthOffset, heightOffset,
                                width, (imH/imW)/(height/width)*height);
                }
                let data = ctx.getImageData(0, 0, width, height).data;
                Module.image_set(
                    enumCode, data, width, height);
            }
            let promiseFunc = () => {
                if (im.width === 0 && im.height === 0) {
                    let p = new Promise(() => setTimeout(promiseFunc, 10));
                    return Promise.resolve(p);
                } else {
                    loadImageToPotentialFunc();
                }
            }
            reader.onloadend = () => {
                let p = new Promise(() => setTimeout(promiseFunc, 10));
                Promise.resolve(p);
            }
            reader.readAsDataURL(uploadImage.files[0]);
        }
    );
}

function modifyUserSliders(enumCode, variableList) {
    if (!(`${enumCode}` in gUserParams))
        gUserParams[`${enumCode}`] = {}; 
    for (let c of variableList) {
        if (!( c in gUserParams[`${enumCode}`]))
            gUserParams[`${enumCode}`][c] = 1.0;
    }
    let userSliders 
        = document.getElementById(`user-sliders-${enumCode}`);
    userSliders.textContent = ``;
    for (let v of variableList) {
        let label = document.createElement("label");
        // label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
        label.textContent = `${v} = ${gUserParams[`${enumCode}`][v]}`;
        userSliders.appendChild(label);
        let slider = document.createElement("input");
        slider.type = "range";
        slider.style = "width: 95%;"
        slider.min = "-5";
        slider.max = "5";
        slider.step = "0.01";
        slider.value = gUserParams[`${enumCode}`][v];
        slider.addEventListener("input", e => {
            let value = Number.parseFloat(e.target.value);
            label.textContent = `${v} = ${value}`;
            gUserParams[`${enumCode}`][v] = value;
            Module.set_user_float_param(enumCode, v, value);
        });
        userSliders.appendChild(document.createElement("br"));
        userSliders.appendChild(slider);
        userSliders.appendChild(document.createElement("br"));
    }
}

function createEntryBoxes(
    controls, enumCode, entryBoxName, count, subLabels
) {
    let label = document.createElement("label");
    // label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = entryBoxName;
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
    let entryBoxes = [];
    for (let i = 0; i < count; i++) {
        let entryBox = document.createElement('input');
        entryBox.type = "text";
        entryBox.value = "";
        entryBox.id = `entry-box-${enumCode}-${i}`;
        entryBox.style = "width: 95%;";
        let label = document.createElement("label");
        // label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
        label.textContent = `${subLabels[i]}`;
        if (count >= 2) {
            controls.appendChild(label);
            controls.appendChild(document.createElement("br"));
        }
        controls.appendChild(entryBox);
        controls.appendChild(document.createElement("br"));
        entryBoxes.push(entryBox);
        entryBox.addEventListener("input", e =>
            Module.set_string_param(enumCode, i, `${e.target.value}`)
        );
    }
    let userSlidersDiv = document.createElement("div");
    userSlidersDiv.id = `user-sliders-${enumCode}`
    controls.appendChild(userSlidersDiv);

}

function createButton(
    controls, enumCode, buttonName, style=''
) {
    let button = document.createElement("button");
    button.innerText = buttonName;
    if (style !== '')
        button.style = style;
    controls.appendChild(button);
    controls.appendChild(document.createElement("br"));
    button.addEventListener("click", e => Module.button_pressed(enumCode));
}

function createLabel(
    controls, enumCode, labelName, style=''
) {
    let label = document.createElement("label");
    if (style !== '')
        label.style = style;
    label.textContent = `${labelName}`;
    label.id = `label-${enumCode}`;
    label.className = 'top-label';
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
}

function createKaTeXLabel(
    controls, enumCode, latexText, style=''
) {
    let label = document.createElement("label");
    if (style !== '')
        label.style = style;
    label.textContent = `${latexText}`;
    label.id = `label-${enumCode}`;
    label.className = 'top-label';
    try {
        katex.render(`\\KaTeX \\space \\text{rendering} \\space \\text{here}.`, 
            label, {
            throwOnError: true
        });
    } catch {

    }
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
}

function editKaTeXLabel(
    enumCode, latexText
) {
    let label = document.getElementById(`label-${enumCode}`);
    try {
        katex.render(latexText, 
            label, {
            throwOnError: true
        });
    } catch {

    }
}

function editLabel(enumCode, textContent) {
    let idVal = `label-${enumCode}`;
    let label = document.getElementById(idVal);
    label.textContent = textContent;
}

function createLineDivider(controls) {
    let hr = document.createElement("hr");
    hr.style = "color:white;"
    controls.appendChild(hr);
}

function createSubDiv(controls, name, style) {
    let div = document.createElement("div");
    let hr = document.createElement("hr");
    hr.style = "color:white;"
    controls.appendChild(hr);
    let label = document.createElement("label");
    if (style !== '')
        label.style = style;
    label.textContent = `+ ${name}`;
    // label.id = `label-${enumCode}`;
    label.className = 'top-label';
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
    controls.appendChild(div);
    div.hidden = true;
    label.addEventListener(
        "click", e => {
            div.hidden = !div.hidden;
            if (label.textContent.at(0) === '+')
                label.textContent 
                    = '-' + label.textContent.substring(1);
            else
                label.textContent 
                    = '+' + label.textContent.substring(1);
        });
    return div;
}

function createHoveringLabelOnCanvas(enumCode, labelContent) {
    let div = document.createElement("div");
    div.style = `position: absolute; z-index: 10;`;
    div.id = `hovering-canvas-label-${enumCode}`;
    let label = document.createElement("label");
    label.textContent = labelContent;
    div.appendChild(label);
    document.getElementById('inner-div2').prepend(div);
}


function editHoveringCanvasLabelTextContent(
    enumCode, textContent) {
    let idVal = `hovering-canvas-label-${enumCode}`;
    let label = document.getElementById(idVal);
    label.textContent = textContent;
}

function editHoveringCanvasVisibilityTopLeftOffset(
    enumCode, isVisible, xPerc, yPerc
) {
    let idVal = `hovering-canvas-label-${enumCode}`;
    let label = document.getElementById(idVal);
    label.style['left'] = `${xPerc}%`;
    label.style['top'] = `${yPerc}%`;
    label.style['visibility'] = (isVisible)? 'visible': 'hidden';
}

function createLinkedLabel(controls, enumCode, labelContent, href) {
    let label = document.createElement("a");
    label.href = href;
    label.textContent = `${labelContent}`;
    label.id = `label-${enumCode}`;
    label.className = 'link-label';
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
}

let controls = document.getElementById('controls');
createLinkedLabel(controls, 0, "Source", "https://github.com/marl0ny/SomeGraphicsStuff/tree/master/OpenGL/RenderIn3D");
createScalarParameterSlider(controls, 2, "Scale", "float", {'value': 10.0, 'min': 0.0, 'max': 20.0, 'step': 0.01});
createVectorParameterSliders(controls, 3, "Domain dimensions", "Vec3", {'value': [128.0, 128.0, 128.0], 'min': [32.0, 32.0, 32.0], 'max': [512.0, 512.0, 512.0], 'step': [0.1, 0.1, 0.1]});
createVectorParameterSliders(controls, 4, "Discretization dimensions", "IVec3", {'value': [64, 64, 64], 'min': [32, 32, 32], 'max': [512, 512, 512], 'step': [2, 2, 4]});
createSelectionList(controls, 5, 2, "Presets", [ "exp(-0.5*x^2/(10.0)^2)*sin(z/4.0)*sin(y/4.0)/(z*y)",  "20.0*exp(0.0-0.5*((x/(sx*10.0))^2 + (y/(sy*10.0))^2))",  "(x + i*y)^8*exp(-(x^2 + y^2 + z^2)/100)*(z/depth)^6",  "exp(-0.5*((x/(sx*10.0))^2 + (y/(sy*15.0))^2 + (z/(sz*10.0))^2))",  "a*sin(x/10)*sin(y/10)*sin(z/10)",  "step(sqrt(x^2 + y^2 + z^2) - 80)",  "1 - step(x - 30) - step(-x - 30)",  "abs(cos(k*x*y*z^2/1500000))^100",  "log(abs(x/10))*log(abs(y/10))*log(abs(z/10))/10",  "cos(10*x*y*z/100000)^3",  "exp(-sqrt((x/5)^2 + (y/5)^2 + (z/5)^2))*(z + x)",  "exp(-0.5*((x-x0)^2 + (y-y0)^2 + (z - z0)^2)/(s*15)^2)*exp(-i*(nx*x/50 + ny*y/50 + nz*z/50))",  "(x+ i*y)^8*exp(-(x^2 + y^2 + z^2)/100)*(z/depth)^6*exp(-f*i*t)",  "exp(-0.5*z^2/(sz*4)^2) - exp(-0.5*y^2/(sy*4)^2) - i*exp(-0.5*x^2/(sx*4)^2)",  "(x+i*y)^18/(x^2 + y^2)*exp(-(x^2+y^2 + z^2)/100)*(z/depth)^16*exp(-i*f*t)"]);
createEntryBoxes(controls, 6, "Enter function f(x, y, z)", 1, []);
createKaTeXLabel(controls, 7, "KaTeX Label");
createSelectionList(controls, 8, 0, "Visualization select", [ "Volume render",  "Three orthogonal planar slices",  "Vector field",  "Three orthogonal planar slices,  vector field",  "Volume render,  vector field"]);
createCheckbox(controls, 9, "Use perspective projection", false);
let subControls0 = createSubDiv(controls, "Volume Render Controls", "");
createCheckbox(subControls0, 11, "Linear interpolation", false);
createScalarParameterSlider(subControls0, 12, "Alpha brightness", "float", {'value': 2.0, 'min': 0.0, 'max': 10.0, 'step': 0.01});
createScalarParameterSlider(subControls0, 13, "Color brightness", "float", {'value': 1.0, 'min': 0.0, 'max': 10.0, 'step': 0.01});
createVectorParameterSliders(subControls0, 14, "Volume dimensions", "IVec3", {'value': [128, 128, 192], 'min': [16, 16, 16], 'max': [512, 512, 512], 'step': [2, 2, 4]});
createCheckbox(subControls0, 15, "Enable bloom", true);
createScalarParameterSlider(subControls0, 16, "Bloominess", "int", {'value': 5, 'min': 0, 'max': 10});
let subControls1 = createSubDiv(controls, "Three Orthogonal Planar Slices Controls", "");
createVectorParameterSliders(subControls1, 19, "Planar slices offsets (in normalized coordinates) for xy, yz, xz", "Vec3", {'value': [0.5, 0.5, 0.5], 'min': [0.0, 0.0, 0.0], 'max': [1.0, 1.0, 1.0], 'step': [0.001, 0.001, 0.001]});
let subControls2 = createSubDiv(controls, "Arrows Plot", "");
createVectorParameterSliders(subControls2, 22, "Arrows dimensions", "IVec3", {'value': [8, 8, 8], 'min': [8, 8, 8], 'max': [128, 128, 128]});
createCheckbox(subControls2, 23, "Use conical arrows", false);
createHoveringLabelOnCanvas(25, "");
