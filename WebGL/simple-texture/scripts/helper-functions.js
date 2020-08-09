

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


function subtract(v, w) {
    return [v[0] - w[0], v[1] - w[1], v[2] - w[2]];
}


function normalize(v) {
    let v2 = v.map(e => e**2);
    let r = Math.sqrt(v2.reduce((a, b) => a + b));
    return v.map(e => e/r);
}


function makeCylinder(height, radius, 
                      numberOfPointsOnEdge) {
    let vertices = [];
    let h = height;
    let r = radius;
    for (var i = 0; i < numberOfPointsOnEdge; i++) {
        let angle = 2.0*Math.PI*i/numberOfPointsOnEdge;
        let deltaAngle = 2.0*Math.PI/numberOfPointsOnEdge;
        let textureMapValue = (angle % (2*Math.PI/6.0))/(2*Math.PI/6.0);
        let c = Math.cos(angle);
        let s = Math.sin(angle);
        let v1 = [
            r*c, r*s, -h/2.0, 1.0, 
            (deltaAngle > 1.0)? 0.0: textureMapValue, 0.0
        ];
        let v2 = [
            r*c, r*s, h/2.0, 1.0, 
            (deltaAngle > 1.0)? 0.0: textureMapValue, 1.0
        ];
        angle = 2.0*Math.PI*(i+1)/numberOfPointsOnEdge;
        textureMapValue = (angle % (2*Math.PI/6.0))/(2*Math.PI/6.0);
        c = Math.cos(angle);
        s = Math.sin(angle);
        let v3 = [
            r*c, r*s, -h/2.0, 1.0, 
            (deltaAngle > 1.0)? 1.0: textureMapValue, 0.0
        ];
        let v4 = [
            r*c, r*s, h/2.0, 1.0, 
            (deltaAngle > 1.0)? 1.0: textureMapValue, 1.0
        ];
        let v2ToV1 = subtract(v1, v2);
        let v1ToV3 = subtract(v3, v1);
        let norm = crossProduct(v2ToV1, v1ToV3);
        let color = [0.0, 0.0, 1.0, 1.0];
        for (let vertex of [v1, v2, v3, v4]) {
            norm.forEach(e => vertex.push(e));
            color.forEach(e => vertex.push(e));
        }
        for (let vertex of [v1, v2, v3, v3, v4, v2]) {
            vertex.forEach(e => vertices.push(e));
        }
    }
    let is2 = 1.0/Math.sqrt(2);
    for (var i = 0; i < numberOfPointsOnEdge; i++) {
        for (var z of [-h/2.0, h/2.0]) {
            let angle = 2.0*Math.PI*i/numberOfPointsOnEdge;
            let c = Math.cos(angle);
            let s = Math.sin(angle);
            let v1 = [
                r*c, r*s, z, 1.0, 
                0.5*(c*is2 - s*is2 + 1), 0.5*(s*is2 + c*is2 + 1),
            ];
            let v2 = [
                0.0, 0.0, z, 1.0, 
                0.5, 0.5,
            ];
            angle = 2.0*Math.PI*(i+1)/numberOfPointsOnEdge;
            c = Math.cos(angle);
            s = Math.sin(angle);
            let v3 = [
                r*c, r*s, z, 1.0, 
                0.5*(c*is2 - s*is2 + 1), 0.5*(s*is2 + c*is2 + 1),
            ];
            let v1ToV2 = subtract(v2, v1);
            let v2ToV3 = subtract(v3, v2);  
            let norm = crossProduct(v1ToV2, v2ToV3);
            norm = normalize(norm);
            if (z > 0) {
                norm = norm.map(e => -e);
            }
            let color = [0.0, 0.0, 1.0, 1.0];
            for (let vertex of [v1, v2, v3]) {
                norm.forEach(e => vertex.push(e));
                color.forEach(e => vertex.push(e));
                vertex.forEach(e => vertices.push(e));
            }
        }
    }
    return new Float32Array(vertices);
}


export {quaternionMultiply, rotationAxisToQuaternion, crossProduct, makeCylinder};