import {crossProduct, 
        normalize, subtract} from "./math-functions.js"
import {TextureSelect} from "./texture-select.js"


function makeCylinder(height, radius, 
                      numberOfPointsOnEdge) {
    let vertices = [];
    let h = height;
    let r = radius;
    for (var i = 0; i < numberOfPointsOnEdge; i++) {
        let angle = 2.0*Math.PI*i/numberOfPointsOnEdge;
        let deltaAngle = 2.0*Math.PI/numberOfPointsOnEdge;
        let t = new TextureSelect(0.5, 0.0, 0.9, 0.5);
        let textureMapValue = t.getX((angle % (2*Math.PI/6.0))/(2*Math.PI/6.0));
        let c = Math.cos(angle);
        let s = Math.sin(angle);
        let v1 = [
            r*c, -h/2.0, r*s, 1.0, 
            (deltaAngle > 1.0)? t.getX(0.0): textureMapValue, t.getY(0.0)
        ];
        let v2 = [
            r*c, h/2.0, r*s, 1.0, 
            (deltaAngle > 1.0)? t.getX(0.0): textureMapValue, t.getY(1.0)
        ];
        angle = 2.0*Math.PI*(i+1)/numberOfPointsOnEdge;
        textureMapValue = t.getX((angle % (2*Math.PI/6.0))/(2*Math.PI/6.0));
        c = Math.cos(angle);
        s = Math.sin(angle);
        let v3 = [
            r*c, -h/2.0, r*s, 1.0, 
            (deltaAngle > 1.0)? t.getX(1.0): textureMapValue, t.getY(0.0) 
        ];
        let v4 = [
            r*c, h/2.0, r*s, 1.0, 
            (deltaAngle > 1.0)? t.getX(1.0): textureMapValue, t.getY(1.0)
        ];
        let v2ToV1 = subtract(v1, v2);
        let v1ToV3 = subtract(v3, v1);
        let norm = crossProduct(v1ToV3, v2ToV1);
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
        // let t = new TextureSelect(0.0, 0.5, 0.5, 1.0);
        let t = new TextureSelect(0.0, 0.0, 0.5, 0.5);
        for (var z of [-h/2.0, h/2.0]) {
            let angle = 2.0*Math.PI*i/numberOfPointsOnEdge;
            let c = Math.cos(angle);
            let s = Math.sin(angle);
            let v1 = [
                r*c, z, r*s, 1.0, 
                t.getX(0.5*(c*is2 - s*is2 + 1)), 
                t.getY(0.5*(s*is2 + c*is2 + 1)),
            ];
            let v2 = [
                0.0, z, 0.0, 1.0, 
                t.getX(0.5), t.getY(0.5),
            ];
            angle = 2.0*Math.PI*(i+1)/numberOfPointsOnEdge;
            c = Math.cos(angle);
            s = Math.sin(angle);
            let v3 = [
                r*c, z, r*s, 1.0, 
                t.getX(0.5*(c*is2 - s*is2 + 1)), 
                t.getY(0.5*(s*is2 + c*is2 + 1)),
            ];
            let v1ToV2 = subtract(v2, v1);
            let v2ToV3 = subtract(v3, v2);  
            let norm = crossProduct(v2ToV3, v1ToV2);
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
    return vertices;
}

export {makeCylinder}