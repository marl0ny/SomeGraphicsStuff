

function add(v, w) {
    let z = [];
    for (let i = 0; i < v.length; i++) {
        z.push(v[i] + w[i]);
    }
    return z;
}


function subtract(v, w) {
    let z = [];
    for (let i = 0; i < v.length; i++) {
        z.push(v[i] - w[i]);
    }
    return z;
}


function normalize(v) {
    let v2 = v.map(e => e**2);
    let r = Math.sqrt(v2.reduce((a, b) => a + b));
    return v.map(e => e/r);
}


function dot(a, b) {
    let sum = 0.0;
    for (let i = 0; i < a.length; i++) {
        sum += a[i]*b[i];
    }
    return sum;
}


function crossProduct(v, w) {
    return [
        v[1]*w[2] - v[2]*w[1],
        v[2]*w[0] - v[0]*w[2],
        v[0]*w[1] - v[1]*w[0]
    ]
}

function quaternionMultiply(q1, q2) {
    let q3 = [0.0, 0.0, 0.0, 0.0];
    q3[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    q3[1] = q1[0]*q2[1] + q1[1]*q2[0] + q1[2]*q2[3] - q1[3]*q2[2]; 
    q3[2] = q1[0]*q2[2] + q1[2]*q2[0] + q1[3]*q2[1] - q1[1]*q2[3]; 
    q3[3] = q1[0]*q2[3] + q1[3]*q2[0] + q1[1]*q2[2] - q1[2]*q2[1];
    return q3;
}


function quaternionConjugate(q) {
    return [q[0], -q[1], -q[2], -q[3]];
}


function quaternionRotate(qr, v) {
    let qrInv = quaternionConjugate(qr);
    let qv = [1.0, v[0], v[1], v[2]];
    let qvQr = quaternionMultiply(qv, qr);
    let qrInvQvQr = quaternionMultiply(qrInv, qvQr);
    return [qrInvQvQr[1], qrInvQvQr[2], qrInvQvQr[3]];
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


export {quaternionMultiply, rotationAxisToQuaternion, 
        quaternionRotate, quaternionConjugate, crossProduct, dot, add, 
        subtract, normalize};
