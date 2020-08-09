

class TextureSelect {

    constructor(minX, minY, maxX, maxY) {
        this.minX = minX;
        this.minY = minY;
        this.maxX = maxX;
        this.maxY = maxY;
    }

    getX (val) {
        return val*(this.maxX - this.minX) + this.minX;
    }

    getY (val) {
        return val*(this.maxY - this.minY) + this.minY;
    }

}

export {TextureSelect}