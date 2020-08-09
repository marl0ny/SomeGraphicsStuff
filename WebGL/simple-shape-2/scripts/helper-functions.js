

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


export {quaternionMultiply, rotationAxisToQuaternion, crossProduct};

