const ENUM_CODES = {
    T: 0,
    BRIGHTNESS: 1,
    SIMULATION_DIMENSIONS3_D: 2,
    DATA_TEXEL_DIMENSIONS3_D: 3,
    PRESET_FUNCTIONS_DROPDOWN: 4,
    USER_TEXT_ENTRY: 5,
    VISUALIZATION_SELECT: 6,
    VOL_RENDER_LINE_DIV: 7,
    VOLUME_RENDER_TITLE: 8,
    ALPHA_BRIGHTNESS: 9,
    COLOR_BRIGHTNESS: 10,
    VOLUME_TEXEL_DIMENSIONS3_D: 11,
    NOISE_SCALE: 12,
    APPLY_BLUR: 13,
    BLUR_SIZE: 14,
    PLANAR_SLICES_LINE_DIV: 15,
    PLANAR_SLICES_LABEL: 16,
    PLANAR_NORM_COORD_OFFSETS: 17,
    ARROWS3_D_LINE_DIV: 18,
    ARROWS3_D_LABEL: 19,
    ARROW_DIMENSIONS: 20,
    DUMMY_VALUE: 21,
};

function createScalarParameterSlider(
    controls, enumCode, sliderLabelName, type, spec) {
    let label = document.createElement("label");
    label.for = spec['id']
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = `${sliderLabelName} = ${spec.value}`
    controls.appendChild(label);
    let slider = document.createElement("input");
    slider.type = "range";
    slider.style ="width: 95%;"
    for (let k of Object.keys(spec))
        slider[k] = spec[k];
    slider.value = spec.value;
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

gCheckboxXorLists = {};

function createCheckbox(controls, enumCode, name, value, xorListName='') {
    let label = document.createElement("label");
    // label.for = spec['id']
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
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

let gVecParams = {};

function createVectorParameterSliders(
    controls, enumCode, sliderLabelName, type, spec) {
    let label = document.createElement("label");
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = `${sliderLabelName} = (${spec.value})`
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
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
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
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
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

let gUserParams = {};

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
        label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
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
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
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
        label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
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
    if (style === '')
        label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    else
        label.style = style;
    label.textContent = `${labelName}`;
    label.id = `label-${enumCode}`;
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
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

let controls = document.getElementById('controls');
createScalarParameterSlider(controls, 1, "Scale", "float", {'value': 10.0, 'min': 0.0, 'max': 20.0, 'step': 0.01});
createVectorParameterSliders(controls, 2, "Domain dimensions", "Vec3", {'value': [128.0, 128.0, 128.0], 'min': [32.0, 32.0, 32.0], 'max': [512.0, 512.0, 512.0], 'step': [0.1, 0.1, 0.1]});
createVectorParameterSliders(controls, 3, "Discretization dimensions", "IVec3", {'value': [128, 128, 128], 'min': [32, 32, 32], 'max': [512, 512, 512], 'step': [2, 2, 4]});
createSelectionList(controls, 4, 2, "Presets", [ "exp(-0.5*x^2/(10.0)^2)*sin(z/4.0)*sin(y/4.0)/(z*y)",  "20.0*exp(0.0-0.5*((x/(sx*10.0))^2 + (y/(sy*10.0))^2))",  "(x + i*y)^8*exp(-(x^2 + y^2 + z^2)/100)*(z/depth)^6",  "exp(-0.5*((x/(sx*10.0))^2 + (y/(sy*15.0))^2 + (z/(sz*10.0))^2))",  "a*sin(x/10)*sin(y/10)*sin(z/10)",  "step(sqrt(x^2 + y^2 + z^2) - 80)",  "1 - step(x - 30) - step(-x - 30)",  "abs(cos(k*x*y*z^2/1500000))^100",  "log(abs(x/10))*log(abs(y/10))*log(abs(z/10))/10",  "cos(10*x*y*z/100000)^3",  "exp(-sqrt((x/5)^2 + (y/5)^2 + (z/5)^2))*(z + x)",  "exp(-0.5*((x-x0)^2 + (y-y0)^2 + (z - z0)^2)/(s*15)^2)*exp(-i*(nx*x/50 + ny*y/50 + nz*z/50))",  "(x+ i*y)^8*exp(-(x^2 + y^2 + z^2)/100)*(z/depth)^6*exp(-f*i*t)",  "exp(-0.5*z^2/(sz*4)^2) - exp(-0.5*y^2/(sy*4)^2) - i*exp(-0.5*x^2/(sx*4)^2)",  "(x+i*y)^18/(x^2 + y^2)*exp(-(x^2+y^2 + z^2)/100)*(z/depth)^16*exp(-i*f*t)"]);
createEntryBoxes(controls, 5, "Enter function f(x, y, z)", 1, []);
createSelectionList(controls, 6, 0, "Visualization select", [ "Volume render",  "Three orthogonal planar slices",  "Vector field"]);
createLineDivider(controls);
createLabel(controls, 8, "Volume Render Controls", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createScalarParameterSlider(controls, 9, "Alpha brightness", "float", {'value': 1.0, 'min': 0.0, 'max': 10.0, 'step': 0.01});
createScalarParameterSlider(controls, 10, "Color brightness", "float", {'value': 1.0, 'min': 0.0, 'max': 10.0, 'step': 0.01});
createVectorParameterSliders(controls, 11, "Volume dimensions", "IVec3", {'value': [128, 128, 192], 'min': [16, 16, 16], 'max': [512, 512, 512], 'step': [2, 2, 4]});
createScalarParameterSlider(controls, 12, "Noise sampling strength", "float", {'value': 0.25, 'min': 0.0, 'max': 1.5, 'step': 0.01});
createCheckbox(controls, 13, "Apply blur", true);
createScalarParameterSlider(controls, 14, "Size", "int", {'value': 1, 'min': 0, 'max': 10});
createLineDivider(controls);
createLabel(controls, 16, "Three Orthogonal Planar Slices Controls", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createVectorParameterSliders(controls, 17, "Planar slices offsets (in normalized coordinates) for xy, yz, xz", "Vec3", {'value': [0.5, 0.5, 0.5], 'min': [0.0, 0.0, 0.0], 'max': [1.0, 1.0, 1.0], 'step': [0.001, 0.001, 0.001]});
createLineDivider(controls);
createLabel(controls, 19, "Arrows Plot", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createVectorParameterSliders(controls, 20, "Arrows dimensions", "IVec3", {'value': [8, 8, 8], 'min': [8, 8, 8], 'max': [128, 128, 128]});

