/* Compute forces between each of the particles, if they
are assessed to be close enough to each other that they have
non-zero interactions. Compute the external forces acting upon
them as well.

References:

D. Schroeder. "Interactive Molecular Dynamics."
Available: https://physics.weber.edu/schroeder/md/.

D. Schroeder, "Interactive Molecular Dynamics,"
American Journal of Physics, 83, 3, 210 - 218. 2015.
http://dx.doi.org/10.1119/1.4901185

*/

#if (__VERSION__ >= 330) || (defined(GL_ES) && __VERSION__ >= 300)
#define texture2D texture
#else
#define texture texture2D
#endif

#if (__VERSION__ > 120) || defined(GL_ES)
precision highp float;
#endif
    
#if __VERSION__ <= 120
varying vec2 UV;
#define fragColor gl_FragColor
#else
in vec2 UV;
out vec4 fragColor;
#endif

uniform float sigma;
uniform float epsilon;
uniform float gForce;
uniform float wallForce;

uniform vec2 simDimensions2D;

uniform sampler2D cellsTex;
uniform ivec2 gridCellsDimensions2D;
uniform vec2 cellDimensions2D;

uniform sampler2D positionVelocitiesTex;
uniform ivec2 particlesTexDimensions2D;
uniform int maxParticlesPerCellCount;
uniform int neighbourCellCountForForceComputation;


int particleIndexFromTextureCoordinates(vec2 UV) {
    int width = particlesTexDimensions2D[0];
    int height = particlesTexDimensions2D[1];
    int rowCount = int(UV.y*float(height) - 0.5);
    int columnPosition = int(UV.x*float(width) - 0.5); 
    return columnPosition + rowCount*width;
}

vec2 toParticlesTextureCoordinates(int index) {
    int width = particlesTexDimensions2D[0];
    int height = particlesTexDimensions2D[1];
    float x = mod(float(index), float(width));
    float y = floor((float(index))/float(width));
    return vec2(
        (x + 0.5)/float(width), (y + 0.5)/float(height));
}

float getCellIndex(vec2 r) {
    float x = r.x, y = r.y;
    float cellXInd = floor(x/cellDimensions2D.x);
    float cellYInd = floor(y/cellDimensions2D.y);
    return cellXInd + cellYInd*float(gridCellsDimensions2D[0]);
}

vec2 cellCoordinateFromCellIndex(float cellIndex) {
    float gridWidth = float(gridCellsDimensions2D[0]);
    float gridHeight = float(gridCellsDimensions2D[1]);
    float x = mod(float(cellIndex), gridWidth);
    float y = floor((float(cellIndex))/gridHeight);
    return vec2((x + 0.5)/gridWidth, (y + 0.5)/gridHeight);
}

vec2 force(vec2 r0, vec2 r1) {
    vec2 r = r1 - r0;
    float r2 = dot(r, r);
    float r4 = r2*r2;
    float r8 = r4*r4;
    float r14 = r8*r4*r2;
    float sigma12 = pow(sigma, 12.0);
    float sigma6 = pow(sigma, 6.0);
    float reg = 0.0;
    float s1 = (4.0*epsilon)*(-12.0*sigma12/r14);
    float s2 = (4.0*epsilon)*(6.0*sigma6/r8);
    return r*(s1 + s2);
}

vec2 getForces(vec2 cellCoord, vec2 r0, int particleIndex0) {
    vec4 cellTex = texture2D(cellsTex, cellCoord);
    float particleAmount = cellTex[1];
    vec2 forcesSum = vec2(0.0, 0.0);
#if (!defined(GL_ES) && __VERSION__ >= 130) || (defined(GL_ES) && __VERSION__ >= 300)
    for (float i = 0.0; i < particleAmount; i++)
#else
    for (float i = 0.0; i < float(maxParticlesPerCellCount); i++)
#endif
    {
        int particleIndex1 = int(cellTex[0] + i);
        vec2 uv1 = toParticlesTextureCoordinates(particleIndex1);
        vec2 r1 = texture2D(positionVelocitiesTex, uv1).xy;
        vec2 f = force(r0, r1);
        forcesSum += (
            i < particleAmount && particleIndex1 != particleIndex0)? 
            f: vec2(0.0, 0.0);
    }
    return forcesSum;
}

vec2 getGravitationalForce() {
    return vec2(0.0, gForce);
}

vec2 getWallForces(vec2 r) {
    float d = 2.0;
    float lw = d*simDimensions2D[0]/float(gridCellsDimensions2D[0]);
    float rw = simDimensions2D[0]*(
        1.0 - d/float(gridCellsDimensions2D[0]));
    float uw = simDimensions2D[1]*(
        1.0 - d/float(gridCellsDimensions2D[1]));
    float dw = d*simDimensions2D[1]/float(gridCellsDimensions2D[1]);
    vec2 leftWallForce = vec2(
        (r.x < lw)? -wallForce*(r.x - lw): 0.0, 0.0);
    vec2 rightWallForce = vec2(
        (r.x > rw)? -wallForce*(r.x - rw): 0.0, 0.0);
    vec2 ceilForce = vec2(
        0.0, (r.y > uw)? -wallForce*(r.y - uw): 0.0);
    vec2 floorForce = vec2(
        0.0, (r.y < dw)? -wallForce*(r.y - dw): 0.0);
    return leftWallForce + rightWallForce + ceilForce + floorForce;
}

vec2 getAllForcesFrom9Cells(vec2 r, float cellIndex, int particleIndex) {
    float gridCellsW = float(gridCellsDimensions2D[0]);
    float gridCellsH = float(gridCellsDimensions2D[1]);
    vec2 cellCoordC, cellCoordL, cellCoordR, cellCoordU, cellCoordD;
    vec2 cellCoordUL, cellCoordUR, cellCoordDL, cellCoordDR;
    cellCoordC = cellCoordinateFromCellIndex(cellIndex);
    cellCoordL = cellCoordinateFromCellIndex(cellIndex - 1.0);
    cellCoordR = cellCoordinateFromCellIndex(cellIndex + 1.0);
    cellCoordUL 
        = cellCoordinateFromCellIndex(cellIndex + gridCellsW - 1.0);
    cellCoordU = cellCoordinateFromCellIndex(cellIndex + gridCellsW);
    cellCoordUR 
        = cellCoordinateFromCellIndex(cellIndex + gridCellsW + 1.0);
    cellCoordDL 
        = cellCoordinateFromCellIndex(cellIndex - gridCellsW - 1.0);
    cellCoordD = cellCoordinateFromCellIndex(cellIndex - gridCellsW);
    cellCoordDR
        = cellCoordinateFromCellIndex(cellIndex - gridCellsW + 1.0);
    vec2 forces = (
        getForces(cellCoordL, r, particleIndex)
        + getForces(cellCoordC, r, particleIndex)
        + getForces(cellCoordR, r, particleIndex)
        + getForces(cellCoordUL, r, particleIndex)
        + getForces(cellCoordU, r, particleIndex)
        + getForces(cellCoordUR, r, particleIndex)
        + getForces(cellCoordDL, r, particleIndex)
        + getForces(cellCoordD, r, particleIndex)
        + getForces(cellCoordDR, r, particleIndex)
        + getGravitationalForce()
        + getWallForces(r)
    );
    return forces;
}

vec2 getAllForcesFrom4Cells(vec2 r, float cellIndex, int particleIndex) {
    float gridCellsW = float(gridCellsDimensions2D[0]);
    float gridCellsH = float(gridCellsDimensions2D[1]);
    vec2 rCellNormalized = vec2(
        mod(r.x, cellDimensions2D[0])/cellDimensions2D[0],
        mod(r.y, cellDimensions2D[1])/cellDimensions2D[1]);
    vec2 cellCoordTL, cellCoordTR, cellCoordBL, cellCoordBR;
    if (rCellNormalized.x < 0.5 && rCellNormalized.y < 0.5) {
        cellCoordTL = cellCoordinateFromCellIndex(cellIndex - 1.0);
        cellCoordTR = cellCoordinateFromCellIndex(cellIndex);
        cellCoordBL = cellCoordinateFromCellIndex(
            cellIndex - gridCellsW - 1.0);
        cellCoordBR = cellCoordinateFromCellIndex(cellIndex - gridCellsW);
    } else if (rCellNormalized.x < 0.5 && rCellNormalized.y >= 0.5) {
        cellCoordTL = cellCoordinateFromCellIndex(
            cellIndex + gridCellsW - 1.0);
        cellCoordTR = cellCoordinateFromCellIndex(
            cellIndex + gridCellsW);
        cellCoordBL = cellCoordinateFromCellIndex(cellIndex - 1.0);
        cellCoordBR = cellCoordinateFromCellIndex(cellIndex);
    } else if (rCellNormalized.x >= 0.5 && rCellNormalized.y >= 0.5) {
        cellCoordTL = cellCoordinateFromCellIndex(
            cellIndex + gridCellsW);
        cellCoordTR = cellCoordinateFromCellIndex(
            cellIndex + gridCellsW + 1.0);
        cellCoordBL = cellCoordinateFromCellIndex(cellIndex);
        cellCoordBR = cellCoordinateFromCellIndex(cellIndex + 1.0);
    } else if (rCellNormalized.x >= 0.5 && rCellNormalized.y < 0.5) {
        cellCoordTL = cellCoordinateFromCellIndex(cellIndex);
        cellCoordTR = cellCoordinateFromCellIndex(cellIndex + 1.0);
        cellCoordBL = cellCoordinateFromCellIndex(
            cellIndex - gridCellsW);
        cellCoordBR = cellCoordinateFromCellIndex(
            cellIndex - gridCellsW + 1.0);
    }
    return (
        getForces(cellCoordTL, r, particleIndex)
        + getForces(cellCoordTR, r, particleIndex)
        + getForces(cellCoordBL, r, particleIndex)
        + getForces(cellCoordBR, r, particleIndex)
        + getGravitationalForce()
        + getWallForces(r)
    );
}

void main() {
    int particleIndex = particleIndexFromTextureCoordinates(UV);
    vec4 q = texture2D(positionVelocitiesTex, UV);
    vec2 r = q.xy;
    vec2 v = q.zw;
    float cellIndex = getCellIndex(r);
    float gridCellsW = float(gridCellsDimensions2D[0]);
    float gridCellsH = float(gridCellsDimensions2D[1]);
    vec2 forces;
    if (neighbourCellCountForForceComputation < 8)
        forces = getAllForcesFrom4Cells(r, cellIndex, particleIndex);
    else
        forces = getAllForcesFrom9Cells(r, cellIndex, particleIndex);
    fragColor = vec4(forces, forces);
}

/* vec4 getRowSortedTexel(int index) {
    return texture2D(xSortedPositionsTex, toTextureCoordinates(index));
}

vec2 getForcesFromRCell(int index, float cellInd, vec4 texel) {
    int numberOfParticles = texDimensions2D[0]*texDimensions2D[1];
    int forwardIndex = index + 1;
    float forwardRowInd;
    vec2 forceSum = vec2(0.0, 0.0);
    do {
        vec4 forwardTexel = getRowSortedTexel(forwardIndex);
        float forwardRowInd = forwardTexel[2];
        vec2 force = force(texel.xy, forwardTexel.xy);
        if (forwardIndex < numberOfParticles 
            && forwardRowInd <= (cellInd + 1.0))
            forceSum += force;
        forwardIndex++;
    } while (forwardRowInd <= (cellInd + 1.0));
    return forceSum;
}

vec2 getForcesFromLCell(int index, float cellInd, vec4 texel) {
    int numberOfParticles = texDimensions2D[0]*texDimensions2D[1];
    int backwardIndex = index - 1;
    float backwardRowInd;
    vec2 forceSum = vec2(0.0, 0.0);
    do {
        vec4 backwardTexel = getRowSortedTexel(backwardIndex);
        float backwardRowInd = backwardTexel[2];
        vec2 force = force(texel.xy, backwardTexel.xy);
        if (backwardIndex >= 0
            && backwardRowInd >= (cellInd - 1.0))
            forceSum += force;
        backwardIndex--;
    } while (backwardRowInd >= (cellInd - 1.0));
    return forceSum;
}

void main() {
    int index = indexFromTextureCoordinates(UV);
    vec4 texel = getRowSortedTexel(index);
    float rowInd = texel[2];
    vec2 forceSum = vec2(0.0);
    int numberOfParticles = texDimensions2D[0]*texDimensions2D[1];

    getForcesFromRCell(index, rowInd, texel);

    vec2 normCellPos = vec2(
        mod((texel.xy - startPos.xy).x, cellSize.x)/cellSize.x,
        mod((texel.xy - startPos.xy).y, cellSize.y)/cellSize.y);
    if (normCellPos.x < 0.5 && normCellPos.y < 0.5) {
        vec2 forcesL = getForcesFromLCell(index, rowInd, texel);
    } else if (normCellPos.x >= 0.5 && normCellPos.y < 0.5) {

    } else if (normCellPos.x < 0.5 && normCellPos.y >= 0.5) {

    } else if (normCellPos.x >= 0.5 && normCellPos.y >= 0.5) {

    }
}*/